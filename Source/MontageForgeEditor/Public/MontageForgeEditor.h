#pragma once

#include "Modules/ModuleManager.h"

/**
 * Editor half of MontageForge: recipes and the builder.
 *
 * Registers nothing on startup - the builder is an editor subsystem the engine creates, and the
 * recipe is an ordinary data asset.
 */
class FMontageForgeEditorModule : public IModuleInterface
{
};
