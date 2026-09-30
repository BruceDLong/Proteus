#pragma once

#include <algorithm>
#include <functional>
#include <memory>
#include <set>
#include <vector>

namespace proteus4 {

// An opaque piece of mutable evidence: a field, adjacency, or even an absent
// catalogue binding. Tokens belong to one graph context. Read and publication
// callbacks run under that context's lock, so observing and subscribing cannot
// race a producer. They do not hold graph nodes or a second adjacency graph.
class Dependency {
public:
    using Wake = std::function<void()>;
    // A reader of derived evidence retains its producer, even if the cache
    // replaces that certificate before the reader has settled.
    std::weak_ptr<void> evidence;
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
        all=all || other.all;
        if(all) keys.clear();
        else keys.insert(other.keys.begin(),other.keys.end());
    }
};

// Semantic readers call observe() at the point they actually consume data,
// including reads made by goals, edge-follow callbacks and completion checks.
// Unconverted helpers can observe({}) to depend on any publication. A scope is
// synchronous and must not escape a callback or be carried to another thread.
class ReadScope {
    using Reader = std::function<void(const DependencyKey&)>;
    inline static thread_local const Reader* current_ = nullptr;
    Reader reader_;
    const Reader* previous_;
public:
    explicit ReadScope(Reader reader):reader_(std::move(reader)),previous_(current_) {current_=&reader_;}
    ~ReadScope() {current_=previous_;}
    ReadScope(const ReadScope&)=delete;
    ReadScope& operator=(const ReadScope&)=delete;
    static void observe(const DependencyKey& key) {if(current_) (*current_)(key);}
};

// A derived fact outlives the search that proved it. Parents retain required
// child proofs, not flattened copies of all descendant reads. Subscriptions
// remain weak; releasing cached evidence releases this dependency DAG.
class DependencyProof : public std::enable_shared_from_this<DependencyProof> {
    bool valid_ = true;
    DependencyKey changed_ = std::make_shared<Dependency>();
    std::set<DependencyKey> reads_;
    std::shared_ptr<Dependency::Wake> wake_;
    std::vector<std::shared_ptr<DependencyProof>> children_;
    std::vector<std::shared_ptr<void>> evidence_;
    std::vector<std::function<void()>> invalidators_;
    void invalidate() {
        if(!valid_) return;
        valid_=false;
        for(auto& invalidate:invalidators_) invalidate();
        invalidators_.clear();changed_->publish();
    }
public:
    using Ptr = std::shared_ptr<DependencyProof>;
    static Ptr make(const std::set<DependencyKey>& reads, std::vector<Ptr> children = {}) {
        auto proof=Ptr(new DependencyProof());proof->changed_->evidence=proof;
        std::weak_ptr<DependencyProof> weak=proof;
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
            if(auto evidence=key->evidence.lock()) evidence_.push_back(std::move(evidence));
        }
    }
    bool valid() const {ReadScope::observe(changed_);return valid_;}
    void onInvalidated(std::function<void()> action) {
        if(valid_) invalidators_.push_back(std::move(action));else action();
    }
};

} // namespace proteus4
