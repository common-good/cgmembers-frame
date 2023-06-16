<?php
use CG\Util as u;
use CG\Web as w;
use Ratchet\MessageComponentInterface;
use Ratchet\ConnectionInterface;
use Ratchet\Server\IoServer;
use Ratchet\Http\HttpServer;
use Ratchet\WebSocket\WsServer;

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
require_once R_ROOT . '/forms/api.inc'; // for authOk()

global $channel; $channel = TX_SOCKET; // set this even if called from PHP window by admin (must be after bootstrapping)
ignore_user_abort(TRUE); // Allow execution to continue even if the request gets canceled.
set_time_limit(0);
$original_session_saving = \drupal_save_session(); // Prevent session information from being saved while cron is running.
\drupal_save_session(FALSE);
$original_user = $GLOBALS['user']; // Force the current user to anonymous to ensure consistent permissions on cron runs.
$GLOBALS['user'] = \drupal_anonymous_user();

class WebSocketsServer implements MessageComponentInterface {
  protected $clients;
  protected $map; // maps account IDs to connections
  
  public function __construct() {$this->clients = new \SplObjectStorage;}
  public function onOpen(ConnectionInterface $conn) {$this->clients->attach($conn);}
  public function onClose(ConnectionInterface $conn) {$this->clients->detach($conn);}
  public function onError(ConnectionInterface $conn, \Exception $e) {return er($e->getMessage());}

  public function onMessage(ConnectionInterface $from, $msg) {
    if (!$ray = json_decode($msg)) return er("Bad JSON message: " . pr($msg), $from);
    extract(just('op deviceId actorId otherId name action amount purpose note', $ray, NULL)); // op, deviceId, and actorId are always required
    if ($deviceId != bin2hex(R_WORD) and !w\authOk('appSocket', $ray, TRUE)) return;
    
    switch ($op) {
      case 'connect': $this->map[$actorId] = $from; break;
      case 'tell':
        if (!$to = nni($this->map, $otherId)) return;
        $what = t('%amt for %what', 'amt what', u\fmtAmt($amount), $purpose);
        $subs = ray('name action what note', $name, $action, $what, $note);
        $message = $action == 'request' ? t('%name asks you to pay %what. Okay?', $subs)
        : ($action == 'denied' ? t('%name has denied your request to pay %what, because "%note".', $subs)
        : t('%name %action you %what.', $subs)); // paid or charged
        $to->send(json_encode(compact(ray('message action note'))));
        break;
      default: return er('Bad op: ' . pr($op), $from);
    }
  }
}

echo 'Running app websocket switchboard...';
$server = IoServer::factory(new HttpServer(new WsServer(new WebSocketsServer())), SOCKET_PORT);
$server->run();

function er($msg, $conn) {
  echo "An error has occurred: $msg\n";
  $conn->close();
}