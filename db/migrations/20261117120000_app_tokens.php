<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class AppTokens extends AbstractMigration {
  public function change() {
    $t = $this->table('app_tokens', ray('id comment', TRUE, 'revocable sign-in tokens of the store app (one per device sign-in); only a hash of each token is kept'));

    $t->addColumn('uid', 'integer', ray('length null comment', phx::INT_BIG, FALSE, 'record ID of the member who signed in'));
    $t->addColumn('tokenHash', 'string', ray('length null comment', 64, FALSE, 'sha256 of the token (never the token itself)'));
    $t->addColumn('deviceHash', 'string', ray('length null comment', 64, TRUE, 'sha256 of the key the app keeps for this installation (so trust outlasts a sign-out)'));
    $t->addColumn('label', 'string', ray('length null comment', 60, TRUE, 'device name, for the list of signed-in devices'));
    $t->addColumn('trusted', 'integer', ray('length default comment', phx::INT_TINY, '0', 'this device skips the second sign-in step (MFA)'));
    $t->addColumn('created', 'integer', ray('length null comment', phx::INT_BIG, FALSE, 'date/time of sign-in'));
    $t->addColumn('lastUsed', 'integer', ray('length null comment', phx::INT_BIG, FALSE, 'date/time the token last worked'));
    $t->addColumn('expires', 'integer', ray('length null comment', phx::INT_BIG, FALSE, 'date/time the token stops working (slides forward on use)'));
    $t->addColumn('revoked', 'integer', ray('length null comment', phx::INT_BIG, TRUE, 'date/time of sign-out (empty until then)'));

    $t->addIndex(['tokenHash'], ['unique' => TRUE]);
    $t->addIndex(['uid', 'deviceHash']);

    $t->create();

    $c = $this->table('app_codes', ray('id comment', TRUE, 'one-time codes that turn an app token into a live-site session; only a hash of each code is kept'));

    $c->addColumn('uid', 'integer', ray('length null comment', phx::INT_BIG, FALSE, 'record ID of the member'));
    $c->addColumn('account', 'integer', ray('length null comment', phx::INT_BIG, FALSE, 'record ID of the account the member acts for on the live site'));
    $c->addColumn('codeHash', 'string', ray('length null comment', 64, FALSE, 'sha256 of the code'));
    $c->addColumn('dest', 'string', ray('length null comment', 255, FALSE, 'live-site path to open after sign-in'));
    $c->addColumn('expires', 'integer', ray('length null comment', phx::INT_BIG, FALSE, 'date/time the code stops working'));
    $c->addColumn('usedAt', 'integer', ray('length null comment', phx::INT_BIG, TRUE, 'date/time the code was used (empty until then)'));

    $c->addIndex(['codeHash'], ['unique' => TRUE]);

    $c->create();
  }

}
