<?php
namespace CG\Web;
use CG as r;
use CG\DB as db;
use CG\Backend as be;
use CG\Util as u;
use CG\Web as w;
use CG\Vote as v;

include_once R_ROOT . '/rvote/rvote.inc';
include_once R_ROOT . '/rvote/rvote-ballot.inc';
include_once R_ROOT . '/rvote/rvote-results.inc';

/**
 * Show notes for a single proposal report.
 */
function formProposalNotes($form, &$sta, $args = '') {
  extract(just('p', $args));

  if (!$id = @$p or !$project = db\get('project', 'r_proposals', compact('id'))) return w\softErr(t('No such proposal.'));
  $VETO = t('VETO!');

  $sql = <<<EOF
    SELECT CONCAT('(q', -v.option, ') ', v.text, IF(v.isVeto, ' <b class="loud">$VETO</b>', '')) AS notes FROM r_proposals p JOIN r_ballots b ON b.question=-p.id JOIN r_votes v ON v.ballot=b.id WHERE v.text <> '' AND p.id=:p ORDER BY p.project, -v.option
EOF;
  $notes = db\q($sql, compact('p'))->fetchCol() ?: [];
  $criteria = db\q('SELECT id,name FROM r_criteria')->fetchAllKeyed();

  $list = '';
  foreach ($notes as $note) $list .= '<li>' . $note . '</li>';
  $map = [];
  foreach ($criteria as $id => $name) $map["(q$id)"] = "<b>$name?</b>";

  $list = strtr($list, $map);
  $list = item("<ul id=\"proposalNotesList\">$list</ul>"); // , t('Comments:')

  $title = item(t('Project Proposal Comments'));
  $project = item($project, t('Project:'));
  $back = item('<p>' . backButton() . '</p>');

  w\css('rvote.css');
  
  return cgform(compact(ray('title project list back')));
}

/**
 * Return an HTML list of comments on a given proposal.
 */
function proposalComments($pid) {

  return ;
}
