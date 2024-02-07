<?php

declare(strict_types=1);

use Rector\Config\RectorConfig;
use Rector\TypeDeclaration\Rector\ClassMethod\AddVoidReturnTypeWhereNoReturnRector;

return RectorConfig::configure()
    ->withPaths([
        __DIR__ . '/cgmembers',
//        __DIR__ . '/db',
    ])
    // uncomment to reach your current PHP version
    ->withPhpSets()
    ->withSkip([
        __DIR__ . '/cgmembers/modules/system/system.api.php', // gets "unexpected T_STRING, expected (" on line 2065
        __DIR__ . '/cgmembers/gherkin/test-template.php',
        Rector\Php71\Rector\FuncCall\RemoveExtraParametersRector::class,
    ])
    ->withRules([
        AddVoidReturnTypeWhereNoReturnRector::class,
    ]);
