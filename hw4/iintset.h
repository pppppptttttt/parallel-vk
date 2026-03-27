#pragma once

namespace hw4 {
class IIntSet {
public:
    virtual ~IIntSet() = default;
    virtual bool add(int key) = 0;
    virtual bool remove(int key) = 0;
    virtual bool contains(int key) const = 0;
};

}  // namespace hw4
