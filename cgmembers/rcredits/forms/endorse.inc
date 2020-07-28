<?php
namespace CG\Web;

/**
 * Provide a synonym for Invite (because we accidentally put it out in a newsletter).
 */
 
include_once __DIR__ . '/invite.inc';

function formEndorse($form, &$sta, $args = '') {return go("/community/invite/$args");}
