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
 * Provide infrastructure for members to discuss and vote.
 * @param string $args: miscellaneous parameters
 * @param string $qid: account QID when called from rdo() (from email link)
 * In MailChimp, use this link for voting links or buttons: https://new.CommonGood.earth/do/doEvents~*|CODE|*
 
To get a list of proposal comments:

  $sql = <<<EOF
    SELECT p.id, p.project, CONCAT('(', v.option-11, ') ', v.text, IF(v.isVeto, ' VETO!', '')) AS notes FROM r_proposals p JOIN r_ballots b ON b.question=-p.id JOIN r_votes v ON v.ballot=b.id WHERE v.text <> '' AND v.id>2425 ORDER BY p.project, v.option
EOF;
  $q = f('db.q', $sql);
  $oldId = -1;
  while ($row = $q->fetchAssoc()) {
    extract($row);
    if ($id != $oldId) {
      if ($oldId != -1) $res[] = "$name:\n$allNotes";
      $name = $project;
      $allNotes = $notes;
      $oldId = $id;
    } else $allNotes .= " $notes";
  }
  if (@$name) $res[] = "$name\n$allNotes";
  
///  debug(join("\n\n", $res));

For list of scorers:
SELECT fullName, Count(b.id) AS votes FROM `users` u INNER JOIN r_ballots b ON b.voter=u.uid LEFT JOIN r_votes v ON v.ballot=b.id WHERE b.question<0 GROUP BY uid

SELECT fullName, b.id, COUNT(v.id) as votes FROM `users` u INNER JOIN r_ballots b ON b.voter=u.uid LEFT JOIN r_votes v ON v.ballot=b.id GROUP BY b.id ORDER BY COUNT(v.id)
  
Anonymous (originally for Ann Arbor):
SELECT q.text AS question,COUNT(b2.voter) AS votes,o.text AS opt,grade,grademax,v.text AS comment,isVeto FROM r_votes v JOIN r_options o ON o.id=v.option JOIN r_questions q ON q.id=o.question JOIN r_ballots b ON b.id=v.ballot LEFT JOIN r_ballots b2 ON b2.proxy=b.voter WHERE q.event=11 GROUP BY q.id,o.id,b.voter

 */
function formEvents($form, &$sta, $args = '') {
  global $base_url, $mya;
  
  extract(just('question do results eid noProxies qid', $args, NULL));
  if ($mya and !$qid) $args = str_replace('qid=&ecode=', "qid=$mya->mainQid&ecode=$mya->emailCode", $args);
  $mya = w\eLinkAcct($sta, $args, $mya ? 'acct' : 'individual'); // allow any signed in account, so user can switch to personal account, below
  if (!$mya->vote) return softErr(t('You have some steps left to become a %PROJECT voting member. <%a>Click here to continue</a>.', '_a', atag('/scraps/vote')));

  $cttyA = $mya->cttyA;
  if ($aA = r\acct()) $cAdmin = $aA->admManageVote; // get signed-in account, if any

  $title = item(t('Community Democracy') . "<br><small>$cttyA->fullName</small>");
//  $subtitle = item($cttyA->fullName);
  $voting = NULL; // default
  
  $event0 = $eid 
  ? db\get('id AS eid, type, event, details, start, end', 'r_events', 'id=:eid AND ctty=:ctty', ray('eid ctty', $eid, $cttyA->id))
  : v\getCurrentEvent($cttyA->id);
  if ($event0) {
    extract($event0); // $event0 contains $event, so must not have that name (as of PHP7)

    $end = fmtDt($end0 = $end);
    $begun = ($start <= now()); // event has begun

    if (strhas('VG', $type) and $begun) {
      if (now() > $end0 and (!$mya->proSe or $mya->co)) { // "or" is necessary if a voting link is accidentally sent to a company
        $switch = r\changeAcctLink($mya->agentId, t('Switch to your personal account'));
        $msg = $mya->co ? t('Companies cannot vote.') : t('You cannot vote on behalf of someone else.');
        w\err("$msg|$switch.");
        return compact('title');
      }

      $proxLink = $aA == $mya ? '/settings/proxies' : "/prox/page=Proxy&priority=1&allow=individual&qid=$mya->mainQid&ecode=$mya->emailCode";
      $voting = $do ? NULL : ($type == 'V' ? v\votingWelcome($end, $proxLink) : v\gradingWelcome($end, $proxLink)); // end is displayable end date
    }

    $start = $start <= now() ? t('NOW') : fmtDt($start);
    $status0 = t('<b>Status:</b> %event from %start to %end', compact(ray('event start end')));
    if ($details) $status0 .= u\lf2br("\n\n$details");
  } else $status0 = t('There are no upcoming %PROJECT events scheduled in your community at this time.');

  $status = $do ? NULL : item($status0);

  if (FALSE and $mya->community == -26742000000002) {
    $middle = file_get_contents(__DIR__ . '/greenfieldma2017.html');
//    $middle = v\showProposalGrades($mya, $args); // FUDGE for 19 Jan 2017 (pending reworking the overall flow and logic of voting phases)
  } elseif ($do == 'proposals') {
    
  } elseif ($do == 'results') {
    $middle = $type == 'G' ? v\showProposalGrades($eid, $noProxies) : v\showResults($eid, $noProxies);
//    $middle = $type == 'G' ? v\showProposalGrades($eid, $noProxies) : t('The results of the recent vote are being tabulated and will be reported soon.');
  } elseif ($do == 'options') { // UNUSED
    list($middle, $phase, $endDate) = v\questionList($eid, $question, $do);
  } elseif ($do == 'ballot') {
    $middle = v\ballot($mya, $extraFields, $question);
  } elseif ($do == 'done' and $eid) { // @todo: handle proposals too
    $table = "r_votes v INNER JOIN r_ballots b ON v.ballot=b.id INNER JOIN r_questions q ON q.id=b.question";
    $notes = db\col('DISTINCT v.text', $table, "q.event=:eid AND b.voter=:uid AND v.text<>''", ray('uid eid', $mya->id, $eid));
    r\tellCAdmin($mya->fullName . t(' has voted. '), $notes);
    $middle = v\done($mya);
    if (!r\acct()) return doSay($middle, 'err');
  } elseif ($do == 'grade') {
//    if (!$question) $question = db\min('id', 'r_proposals', 'event=:eid', compact('eid'));
    $extraFields = v\showProposal($mya, -$question, $middle, TRUE); // middle here is an error message, if anything
  } else {
    $extraFields = NULL;
    $middle = t('Event History');
    if ($cAdmin) $middle .= ' ' . button(t('Add Event'), "$base_url/community/events/event/do=add", '', 'warning');
    $middle = "<h4>$middle</h4>";
    $sql = <<<EOF
      SELECT id, end, event FROM r_events WHERE ctty=:ctty ORDER BY end DESC, id DESC
EOF;
    $q = db\q($sql, ray('ctty', $cttyA->id));
    $list = '';
    while ($row = $q->fetchAssoc()) {
      extract($row, EXTR_PREFIX_ALL, 'e');
      $buttons = $cAdmin ? button(t('Edit'), "$base_url/community/events/event/do=edit&eid=$e_id", '', 'warning') : '';
      if ($cAdmin or $e_end < now()) {
        $btn = button('%text', "$base_url/community/events/do=results&eid=$e_id%noprox", '', 'success');
        $buttons .= ' &nbsp; ' . tr($btn, 'text noprox', t('Results'), '');
//        if ($cAdmin) $buttons .= ' ' . tr($btn, 'text noprox', t('Without Proxies'), '&noProxies=1');
      }
      $e_end = fmtDt($e_end);
      $list .= <<<EOF
<tr><td>$e_end</td><td>$e_event</td><td>$buttons</td></tr>
EOF;
    }
    if ($list) $list = "<table id=\"event-history\"><tr><th>Ends</th><th>Event</th></tr>\n$list</table>";
    $middle .= $list ?: t('There have not yet been any %PROJECT events in your community.');
  }
  
//  } else $middle = t('There is no vote in progress in your community.');
//  } else $middle = t('There are no questions pending in your community.');
  
  $middle = item(nn($middle));

  w\js('ballot');
  w\css('rvote.css');
  
  u\setDft($extraFields, NULL);
  return cgform(compact(ray('title status voting middle extraFields')));
}

function formEvents_validate($form, &$sta) {
  extract(just('question agree op', $sta['input'], NULL));
  $mya = w\eLinkAcct($sta);
  if ($question < 0 and w\op($sta) == 'submit') {
    // for each option, if blank then error
  }
}

function formEvents_submit($form, &$sta) {
  extract(just('question agree op', $sta['input'], NULL));
  $mya = w\eLinkAcct($sta); $eLink = eLink($sta);
  $ctty = $mya->community;

  $op = (w\op($sta) == 'prev' and strpos($op, t('Previous')) === FALSE) ? 'doneWithQ' : w\op($sta);
  if ($op == 'showVotes') return w\say(v\showVotes($ctty));
  if (in($op, 'noDirect noVote')) return w\say(v\showNonVoters($ctty, $op == 'noDirect'));

  if ($op == 'gradeIt') return w\go("prox/page=Events&do=grade&$eLink");
  if ($agree) return w\go("prox/page=Events&do=ballot&$eLink");
  
  // process ballot submission
  if ($question < 0) {
    $type = 'M';
    $do = 'grade';
    if (is_numeric($question = bumpQnum(-$question, v\getProposals($mya), $op))) $question = -$question;
  } else {
    list($quids, $qutypes) = v\getQuestions('', $mya); 
    $type = $qutypes[$question];
    $do = 'ballot';
    $question = bumpQnum($question, $quids, $op);
  }
  v\processvote($sta['input'], $type); // any errors are handled by javaScript (only the user suffers if no JS)

// redundant  if ($question == 'done') w\say(t('That was the last item. Thank you for participating!'));
  if ($question != 'before') $eLink .= $question == 'done' ? '&do=done' : "&do=$do&question=$question";
  return w\go("prox/page=Events&$eLink"); 
}

/**
 * Get next or previous question number.
 * @param int $question: current question number
 * @param array $quids: array of question numbers
 * @param string $op: submitted operation button
 * @return next question number ('done' if past last, 'before' if before first)
 */
function bumpQnum($question, $quids, $op) {
  $qnum = array_search($question, $quids); // this works even if the number of elections/questions changes during voting
  if (nn($op) == 'prev') $qnum--; else $qnum++;
  return $qnum < 0 ? 'before' : ($qnum >= count($quids) ? 'done' : nni($quids, $qnum));
}
