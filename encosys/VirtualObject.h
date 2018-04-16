#pragma once

namespace ecs {

class VirtualObject {
    struct HolderBase {
        virtual ~HolderBase () {}
        virtual HolderBase* Clone () const = 0;
    };

    template <typename T>
    struct Holder : HolderBase {
        Holder () = default;
        Holder (const T& t) : m_held{t} {}
        virtual ~Holder () {};
        virtual HolderBase* Clone () const { return new Holder<T>(*this); }
        T m_held;
    };

public:
    VirtualObject () = default;
    VirtualObject (const VirtualObject& other) : m_storage{other.m_storage->Clone()} {}
    template <typename T> VirtualObject (const T& t) : m_storage{new Holder<T>(t)} {}
    ~VirtualObject () { Clear(); }

    VirtualObject& operator= (const VirtualObject& other) {
        if (this != &other) {
            Clear();
            m_storage = other.m_storage->Clone();
        }
        return *this;
    }

    template <typename T>
    VirtualObject& operator= (const T& t) {
        Clear();
        m_storage = new Holder<T>(t);
        return *this;
    }

    void Clear () {
        if (m_storage) {
            delete m_storage;
            m_storage = nullptr;
        }
    }

    template <typename T> T& Get () { return static_cast<Holder<T>*>(m_storage)->m_held; }
    template <typename T> const T& Get () const { return static_cast<Holder<T>*>(m_storage)->m_held; }

private:
    HolderBase* m_storage{nullptr};
};

} // namespace ecs