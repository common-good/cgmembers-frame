<?php
%FEATURE_HEADER
require_once __DIR__ . '/../%MODULE.steps';

class %MODULE%FEATURE_NAME {
  var $sceneName;
  var $step;
  var $isThen;

  public function setUp($sceneName, $variant = '') {
    global $sceneTest; $sceneTest = $this;
    global $testModule; $testModule = '%MODULE';
    global $testOnly;

    $this->sceneName = $sceneName;
    if (function_exists('extraSetup')) extraSetup($this); // defined in %MODULE.steps
    $this->sceneName .= ' Setup';

    switch ($variant) {
      default: // fall through to case(0)
%SETUP_LINES
    }
    $this->sceneName = $sceneName;
  }
%TESTS
}