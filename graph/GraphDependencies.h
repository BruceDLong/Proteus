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
private:
    std::vector<std::weak_ptr<Wake>> readers_;
public:
    void subscribe(const std::shared_ptr<Wake>& reader) {
        readers_.erase(std::remove_if(readers_.begin(),readers_.end(),
            [](const auto& weak) {return weak.expired();}),readers_.end());
        readers_.push_back(reader);
    }
    void publish() {
        readers_.erase(std::remove_if(readers_.begin(),readers_.end(),
            [](const auto& weak) {
                if(auto reader=weak.lock()) {(*reader)();return false;}
                return true;
            }),readers_.end());
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

} // namespace proteus4
