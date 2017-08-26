#include "SystemRegistry.h"

#include "System.h"

namespace ecs {

SystemRegistry::~SystemRegistry () {
    for (System* system : m_systems) {
        delete system;
    }
}

}