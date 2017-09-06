#pragma once

#include "EncosysDefines.h"

namespace ecs {

enum class Existence {
    Required,
    Optional
};

enum class Access {
    Read,
    Write
};

template <typename TComponent, Existence existence, Access access>
class ComponentDependency {
};

namespace tmp {

// Check if T is a ComponentDependency
template <typename T>
struct IsComponentDependency {
    static constexpr bool value = false;
};
template <typename TComponent, Existence existence, Access access>
struct IsComponentDependency<ComponentDependency<TComponent, existence, access>> {
    static constexpr bool value = true;
};

// Check if TComponent matches the TDependency and has read access
template <typename TComponent, typename TDependency>
struct DependencyHasReadAccess {
    static constexpr bool value = false;
};
template <typename TComponent, Existence existence>
struct DependencyHasReadAccess<TComponent, ComponentDependency<TComponent, existence, Access::Read>> {
    static constexpr bool value = true;
};
template <typename TComponent>
struct ComponentHasReadAccess {
    template <typename TDependency>
    using Filter = DependencyHasReadAccess<TComponent, TDependency>;
};

// Check if TComponent matches the TDependency and has write access
template <typename TComponent, typename TDependency>
struct DependencyHasWriteAccess {
    static constexpr bool value = false;
};
template <typename TComponent, Existence existence>
struct DependencyHasWriteAccess<TComponent, ComponentDependency<TComponent, existence, Access::Write>> {
    static constexpr bool value = true;
};
template <typename TComponent>
struct ComponentHasWriteAccess {
    template <typename TDependency>
    using Filter = DependencyHasWriteAccess<TComponent, TDependency>;
};

}

}