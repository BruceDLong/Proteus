#pragma once

#include <algorithm>
#include <functional>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <vector>

namespace proteus4 {

// An opaque piece of mutable evidence: a field, adjacency, or even an absent
// catalogue binding. Tokens belong to one graph context. Read and publication
// callbacks run under that context's lock, so observing and subscribing cannot
// race a producer. They do not hold graph nodes or a second adjacency graph.
class DependencyProof;
class Dependency {
public:
    using Wake = std::function<void()>;
    // A reader of derived evidence retains its producer, even if the cache
    // replaces that certificate before the reader has settled.
    std::weak_ptr<void> evidence;
    std::weak_ptr<DependencyProof> proof; // typed provenance for pending-write checks
    // Completed immutable facts ignore ordinary progress notifications. New
    // obligations have a separate token; derived readers preserve that DAG.
    std::shared_ptr<Dependency> requirements;
    bool completionRelevant = false;
    std::function<std::shared_ptr<DependencyProof>()> completedEvidence;
private:
    std::vector<std::weak_ptr<Wake>> readers_;
    std::size_t cleanupAt_ = 64;
public:
    void subscribe(const std::shared_ptr<Wake>& reader) {
        // Many goals may observe the same fact. Compact in batches, rather
        // than rescanning every live reader for each new subscription.
        if(readers_.size()>=cleanupAt_) {
            readers_.erase(std::remove_if(readers_.begin(),readers_.end(),
                [](const auto& weak) {return weak.expired();}),readers_.end());
            cleanupAt_=std::max<std::size_t>(64,2*readers_.size());
        }
        readers_.push_back(reader);
    }
    void publish() {
        readers_.erase(std::remove_if(readers_.begin(),readers_.end(),
            [](const auto& weak) {
                if(auto reader=weak.lock()) {(*reader)();return false;}
                return true;
            }),readers_.end());
        cleanupAt_=std::max<std::size_t>(64,2*readers_.size());
    }
};
using DependencyKey = std::shared_ptr<Dependency>;

// Unknown footprints retain the conservative behaviour. An empty, explicit
// footprint is a metadata-only change and does not advance semantic revision.
struct Changes {
    bool all = true;
    bool objectWrites = false; // adapter data writes, as opposed to proof-cache updates
    std::set<DependencyKey> keys;
    static Changes only(std::initializer_list<DependencyKey> keys) {
        Changes result;result.all=false;
        for(const auto& key:keys) {
            if(!key) {result.all=true;result.keys.clear();break;}
            result.keys.insert(key);
        }
        return result;
    }
    bool empty() const {return !all && keys.empty();}
    void include(const Changes& other) {
        objectWrites=objectWrites || other.objectWrites;
        all=all || other.all;
        if(all) keys.clear();
        else keys.insert(other.keys.begin(),other.keys.end());
    }
};

// Per-context evidence tokens for addresses inspected by the adapter. These
// are subscriptions, not graph nodes or adjacency. Keys never retain model
// objects. Address reuse can only over-invalidate; unused tokens are reclaimed.
class ObjectDependencies {
    std::unordered_map<const void*,DependencyKey> keys_;
    std::size_t cleanupAt_=256;
public:
    DependencyKey key(const void* object) {
        if(!object) return {};
        auto found=keys_.find(object);
        if(found!=keys_.end()) return found->second;
        if(keys_.size()>=cleanupAt_) {
            for(auto it=keys_.begin();it!=keys_.end();)
                if(it->second.use_count()==1 && it->second->requirements.use_count()==1) it=keys_.erase(it);else ++it;
            cleanupAt_=std::max<std::size_t>(256,2*keys_.size());
        }
        auto key=std::make_shared<Dependency>();
        key->requirements=std::make_shared<Dependency>();
        key->requirements->completionRelevant=true;
        return keys_[object]=std::move(key);
    }
};

// Semantic readers call observe() at the point they actually consume data,
// including reads made by goals, edge-follow callbacks and completion checks.
// Unconverted helpers can observe({}) to depend on any publication. A scope is
// synchronous and must not escape a callback or be carried to another thread.
class ReadScope {
    using Reader = std::function<void(const DependencyKey&)>;
    inline static thread_local ReadScope* current_ = nullptr;
    inline static thread_local ObjectDependencies* objects_ = nullptr;
    ObjectDependencies* previousObjects_;
    Reader reader_;
    ReadScope* previous_;
    std::unordered_set<const void*> observedObjects_;
    const void* recent_[8]{};
    unsigned recentIndex_=0;
public:
    explicit ReadScope(Reader reader,ObjectDependencies* objects=nullptr)
        :previousObjects_(objects_),reader_(std::move(reader)),previous_(current_) {
        current_=this;if(objects) objects_=objects;
    }
    ~ReadScope() {current_=previous_;objects_=previousObjects_;}
    ReadScope(const ReadScope&)=delete;
    ReadScope& operator=(const ReadScope&)=delete;
    static void observe(const DependencyKey& key) {if(current_ && current_->reader_) current_->reader_(key);}
    static void observeObject(const void* object) {
        if(!object || !current_ || !current_->reader_ || !objects_) return;
        // Helpers often inspect the same view many times in one callback.
        // Register it once per reader scope; a nested proof capture has its
        // own set and therefore still records and replays its dependencies.
        auto& scope=*current_;
        for(auto recent:scope.recent_) if(recent==object) return;
        scope.recent_[scope.recentIndex_++ % 8]=object;
        if(scope.observedObjects_.insert(object).second) observe(objects_->key(object));
    }
    static void newRequirements(std::initializer_list<const void*> objects) {
        if(!objects_) return;
        for(auto object:objects) if(object) objects_->key(object)->requirements->publish();
    }
    static bool tracksObjects() {return objects_!=nullptr;}
    static Changes changedObjects(std::initializer_list<const void*> objects) {
        if(!objects_) return {}; // callers outside a tracked search stay conservative
        auto changes=Changes::only({});changes.objectWrites=true;
        for(auto object:objects) if(object) changes.keys.insert(objects_->key(object));
        return changes;
    }
};

// An atomic domain operation can collect indirect writes from lower-level
// helpers. Collection never publishes: the enclosing Set returns the footprint
// after all writes finish. No model objects or graph edges are retained here.
class WriteScope {
    inline static thread_local WriteScope* current_ = nullptr;
    WriteScope* previous_;
    Changes changes_ = Changes::only({});
public:
    WriteScope():previous_(current_) {current_=this;}
    ~WriteScope() {current_=previous_;if(previous_) previous_->changes_.include(changes_);}
    WriteScope(const WriteScope&)=delete;
    WriteScope& operator=(const WriteScope&)=delete;
    static void record(const Changes& changes) {if(current_) current_->changes_.include(changes);}
    const Changes& changes() const {return changes_;}
};

// A derived fact outlives the search that proved it. Parents retain required
// child proofs, not flattened copies of all descendant reads. Subscriptions
// remain weak; releasing cached evidence releases this dependency DAG.
class DependencyProof : public std::enable_shared_from_this<DependencyProof> {
    bool valid_ = true;
    bool completionOnly_ = false;
    std::shared_ptr<DependencyProof> completion_;
    DependencyKey changed_ = std::make_shared<Dependency>();
    std::set<DependencyKey> reads_;
    std::shared_ptr<Dependency::Wake> wake_;
    std::vector<std::shared_ptr<DependencyProof>> children_;
    std::vector<std::shared_ptr<void>> evidence_;
    std::vector<DependencyKey> derived_;
    std::vector<std::function<void()>> invalidators_;
    void observeCompletion(const DependencyKey& key) {
        if(key->completionRelevant) {
            completion_->observe({key});
            if(key->completedEvidence) if(auto source=key->completedEvidence())
                if(!source->live()) completion_->invalidate();
        }
        else if(key->requirements) completion_->observe({key->requirements});
        else if(key->completedEvidence) {
            if(auto source=key->completedEvidence()) {
                completion_->observe({source->changed_});
                if(!source->live()) completion_->invalidate();
            }
        }
    }
    void invalidate() {
        if(!valid_) return;
        valid_=false;
        for(auto& invalidate:invalidators_) invalidate();
        invalidators_.clear();changed_->publish();
    }
public:
    using Ptr = std::shared_ptr<DependencyProof>;
    static Ptr make(const std::set<DependencyKey>& reads, std::vector<Ptr> children = {}) {
        auto proof=Ptr(new DependencyProof());proof->changed_->evidence=proof;proof->changed_->proof=proof;
        std::weak_ptr<DependencyProof> weak=proof;
        proof->changed_->completedEvidence=[weak] {
            auto source=weak.lock();return source ? source->completion() : Ptr{};
        };
        proof->wake_=std::make_shared<Dependency::Wake>([weak] {if(auto p=weak.lock()) p->invalidate();});
        proof->children_=std::move(children);proof->observe(reads);
        for(const auto& child:proof->children_) {
            proof->observe({child->changed_});
            if(!child->valid_) proof->invalidate();
        }
        return proof;
    }
    void observe(const std::set<DependencyKey>& reads) {
        for(const auto& key:reads) if(key!=changed_ && reads_.insert(key).second) {
            key->subscribe(wake_);
            if(completion_) observeCompletion(key);
            if(auto evidence=key->evidence.lock()) {
                evidence_.push_back(std::move(evidence));derived_.push_back(key);
            }
        }
    }
    // Project a settled proof onto obligations, retaining child proofs rather
    // than flattening their reads. Broad wakeups and acquired facts are absent.
    Ptr completion() {
        if(completionOnly_) return shared_from_this();
        if(!completion_) {
            completion_=make({});completion_->completionOnly_=true;
            completion_->changed_->completionRelevant=true;
            for(const auto& key:reads_) observeCompletion(key);
        }
        return completion_;
    }
    // Query an unpublished footprint without invalidating or publishing it.
    // Opaque derived evidence has no inspectable footprint and stays conservative.
    bool affectedBy(const Changes& changes) const {
        if(changes.empty()) return false;
        if(changes.all) return true;
        std::vector<const DependencyProof*> pending{this};
        std::unordered_set<const DependencyProof*> seen;
        while(!pending.empty()) {
            auto at=pending.back();pending.pop_back();
            if(!seen.insert(at).second) continue;
            for(const auto& key:changes.keys) if(at->reads_.count(key)) return true;
            for(const auto& key:at->derived_) {
                auto source=key->proof.lock();
                if(!source) return true;
                // The existing evidence_ ownership retains each source proof.
                pending.push_back(source.get());
            }
        }
        return false;
    }
    bool observes(const DependencyKey& key) const {return reads_.count(key)!=0;}
    bool live() const {return valid_;} // Cache maintenance must not record a semantic read.
    bool valid() const {ReadScope::observe(changed_);return valid_;}
    void onInvalidated(std::function<void()> action) {
        if(valid_) invalidators_.push_back(std::move(action));else action();
    }
};

} // namespace proteus4
