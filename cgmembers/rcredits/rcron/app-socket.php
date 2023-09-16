<?php
use CG\Util as u;
use CG\Web as w;
use CG\DB as db;
use CG\QR as qr;
use Ratchet\MessageComponentInterface;
use Ratchet\ConnectionInterface;
use Ratchet\Server\IoServer;
use Ratchet\Http\HttpServer;
use Ratchet\WebSocket\WsServer;
use React\Socket\Server;
use React\Socket\SecureServer;

/**
 * @file
 * Switchboard to route messages from one instance of the CGPay app to another.
 * Most obviously: "I request that you pay me $x for whatever." (and the yes/no response)
 *
 * Parameters for messaging:
 *   op deviceId actorId otherId action amount description created note
 *   op: conn or tell
 *   deviceId: the device's assigned ID (must be associated in r_boxes with actorId)
 *   actorId: the sender's QR-style account ID
 *   otherId: recipient account ID
 *   action: paid, charged, request, denied
 *   amount: transaction or request amount
 *   description: transaction (or request) description
 *   created: transaction or invoice creation date
 *   note: if action is "denied", this is the reason
 */
define('DRUPAL_ROOT', __DIR__ . '/../..');
require_once __DIR__ . '/../bootstrap.inc';
require_once DRUPAL_ROOT . '/../vendor/autoload.php';
drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL); // boot before including rcron.inc
require_once R_ROOT . '/cg-qr.inc';

global $channel; $channel = TX_SOCKET; // set this even if called from PHP window by admin (must be after bootstrapping)
ignore_user_abort(TRUE); // Allow execution to continue even if the request gets canceled.
set_time_limit(0);
$original_session_saving = \drupal_save_session(); // Prevent session information from being saved while cron is running.
\drupal_save_session(FALSE);
$original_user = $GLOBALS['user']; // Force the current user to anonymous to ensure consistent permissions on cron runs.
$GLOBALS['user'] = \drupal_anonymous_user();

class MyWSSServer implements MessageComponentInterface {
  protected $clients;
  protected $map; // maps account IDs to connections
  
  public function __construct() {$this->clients = new \SplObjectStorage;}
  public function onOpen(ConnectionInterface $conn) {$this->clients->attach($conn);}
  public function onClose(ConnectionInterface $conn) {$this->clients->detach($conn);}
  public function onError(ConnectionInterface $conn, \Exception $e) {return er($e->getMessage());}

  /**
   * Handle an incoming message.
   * @param conn $from: what connection the message came in on
   * @param string $msg: JSON-encoded message
   */
  public function onMessage(ConnectionInterface $from, $msg) {
    if (!$ray = json_decode($msg)) return er(t('Bad JSON message: ') . pr($msg), $from);
///    flog('app socket got: ' . pr($ray));
    extract(just('op deviceId actorId otherId name action amount purpose note', $ray, NULL)); // op, deviceId, and actorId are always required
    if (!$a = qr\acct($actorId, FALSE)) return er(t('"%actorId" is not a recognized actorId.', compact('actorId')), $from);
    if (!$a->ok) return er(t('%uid is not an active account.', 'uid', $a->id), $from);
    $ok = ( ($deviceId == bin2hex(R_WORD))
    or (db\get('uid', 'r_boxes', ray('code', $deviceId)) == $a->id)
    or ($deviceId == 'dev' . $a->fullName[0] and !isPRODUCTION) ); // for example devA)
    if (!$ok) return er(t('"%actorId" is not an authorized account.', compact('actorId')), $from); // server sends R_WORD instead of deviceId
    
    switch ($op) {
      case 'connect': $this->map[$actorId] = $from; break;
      case 'tell': // currently this comes only from u/tellApp()
        if (!$to = nni($this->map, $otherId)) return;
        $what = t('%amt for %what', 'amt what', u\fmtAmt($amount), $purpose);
        $subs = ray('name action what note', $name, $action, $what, $note);
        $message = $action == 'request' ? t('%name asks you to pay %what. Okay?', $subs)
        : ($action == 'denied' ? t('%name has denied your request to pay %what, because "%note".', $subs)
        : t('%name %action you %what.', $subs)); // paid or charged
        $to->send(json_encode(compact(ray('message action note'))));
        break;
      default: return er(t('Bad op: ') . $op, $from);
    }
  }
}

try {
  set_error_handler(function () { exit(); }, E_WARNING); // ignore warning about "Address already in use"
  flog('Running app websocket switchboard...');
  $loop = \React\EventLoop\Factory::create();
  $websockets = new Server('0.0.0.0:' . SOCKET_PORT, $loop);
  restore_error_handler();
  
  $secure_websockets = new SecureServer($websockets, $loop, [
      'local_cert' => '/etc/pki/tls/certs/commongood.earth.pem',
      'local_pk' => '/etc/pki/tls/private/commongood.earth.key',
      'verify_peer' => false,
  ]);

  $app = new HttpServer(new WsServer(new MyWSSServer()));
  $server = new IoServer($app, $secure_websockets, $loop);
  $server->run();
} catch (\Exception $er) {
  flog("App socket overall er: " . $er->message());
}

function er($msg, $conn) {
/**/ flog("App socket error: $msg\n");
  $conn->close();
}
