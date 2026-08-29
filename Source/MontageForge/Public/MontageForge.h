#pragma once

#include "Modules/ModuleManager.h"
#include "Logging/LogMacros.h"

/** Filter the Output Log on "LogMontageForge" to follow montage building and notify firing. */
MONTAGEFORGE_API DECLARE_LOG_CATEGORY_EXTERN(LogMontageForge, Log, All);

/**
 * Runtime half of MontageForge: the notify, and nothing else.
 *
 * Everything that builds montages is editor-only and lives in MontageForgeEditor. This module is
 * runtime purely because a notify placed in a montage has to exist in a packaged game.
 */
class FMontageForgeModule : public IModuleInterface
{
};
