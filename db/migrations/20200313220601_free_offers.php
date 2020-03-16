<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class FreeOffers extends AbstractMigration {
  /**
   * Change Method.
   */
  public function change() {
    $t = $this->table('post_cats', ['comment' => 'types of offers and needs']);
    $t->addColumn('cat', 'string', ray('length null comment', 255, TRUE, 'category of service'));
    $t->create();
    
    $t->insert(['cat' =>'food']);
    $t->insert(['cat' =>'housing']);
    $t->insert(['cat' =>'clothing']);
    $t->insert(['cat' =>'healthcare']);
    $t->insert(['cat' =>'rides']);
    $t->insert(['cat' =>'delivery']);
    $t->insert(['cat' =>'childcare']);
    $t->insert(['cat' =>'adult care']);
    $t->insert(['cat' =>'animal care']);
    $t->insert(['cat' =>'cleaning']);
    $t->insert(['cat' =>'legal']);
    $t->insert(['cat' =>'social service advice']);
    $t->insert(['cat' =>'fellowship']);
    $t->insert(['cat' =>'muscle']);
    $t->insert(['cat' =>'communication']);
    $t->insert(['cat' =>'technology']);
    $t->insert(['cat' =>'teaching/learning']);
    $t->insert(['cat' =>'other']);
    $t->save();
    
    $t = $this->table('people', ray('id primary_key comment', FALSE, 'pid', 'contact information for non-members'));
    $t->addColumn('pid', 'integer', ray('identity length null comment', TRUE, phx::INT_BIG, TRUE, 'record ID'));
    $t->addColumn('displayName', 'string', ray('length null comment', 255, TRUE, 'first name or nickname'));
    $t->addColumn('fullName', 'string', ray('length null comment', 255, TRUE, 'full name'));
    $t->addColumn('address', 'string', ray('length null comment', 255, TRUE, 'physical address'));
    $t->addColumn('city', 'string', ray('length null comment', 255, TRUE, 'city'));
    $t->addColumn('state', 'integer', ray('length null comment', phx::INT_MEDIUM, TRUE, 'state'));
    $t->addColumn('zip', 'string', ray('length null comment', 255, TRUE, 'postal code'));
    $t->addColumn('phone', 'string', ray('length null comment', 255, TRUE, 'phone number'));
    $t->addColumn('email', 'string', ray('length null comment', 255, TRUE, 'email address'));
    $t->addColumn('method', 'enum', ray('values null comment', ray('email phone text'), TRUE, 'preferred contact method'));
    $t->addColumn('latitude', 'decimal', ray('precision scale default comment', 11, 8, 0, 'latitude of location'));
    $t->addColumn('longitude', 'decimal', ray('precision scale default comment', 11, 8, 0, 'longitude of location'));
    $t->addColumn('confirmed', 'integer', ray('length default comment', phx::INT_TINY, 0, 'confirmed by email'));
    $t->addColumn('created', 'string', ray('length null comment', 11, TRUE, 'creation date'));
    $t->addIndex(['email']);
    $t->create();

    $t = $this->table('messages', ['comment' => 'messages responding to offers and needs']);
    $t->addColumn('postid', 'integer', ray('length null comment', phx::INT_BIG, TRUE, 'related post ID'));
    $t->addColumn('message', 'string', ray('length null comment', 255, TRUE, 'the message'));
    $t->addColumn('from', 'integer', ray('length null comment', phx::INT_BIG, TRUE, 'pid of sender'));
    $t->addColumn('to', 'integer', ray('length null comment', phx::INT_BIG, TRUE, 'pid of recipient'));
    $t->addColumn('confirmed', 'integer', ray('length default comment', phx::INT_TINY, 0, 'confirmed by email'));
    $t->addColumn('created', 'string', ray('length null comment', 11, TRUE, 'creation date'));
    $t->addIndex(['from']);
    $t->addIndex(['to']);
    $t->create();

    $t = $this->table('posts', ['comment' => 'offers and needs posted by members and non-members']);
    $t->addColumn('type', 'enum', ray('values null comment', ray('need offer'), TRUE, 'item type'));
    $t->addColumn('item', 'string', ray('length null comment', 255, TRUE, 'item offered or needed'));
    $t->addColumn('details', 'string', ray('length null comment', 255, TRUE, 'description of item'));
    $t->addColumn('cat', 'integer', ray('length default comment', phx::INT_TINY, 0, 'service category'));
    $t->addColumn('exchange', 'integer', ray('length default comment', phx::INT_TINY, 0, 'is exchange wanted?'));
    $t->addColumn('emergency', 'integer', ray('length default comment', phx::INT_TINY, 0, 'because of a short-term emergency'));
    $t->addColumn('confirmed', 'integer', ray('length default comment', phx::INT_TINY, 0, 'confirmed by email'));
    $t->addColumn('uid', 'integer', ray('length null comment', phx::INT_BIG, TRUE, "poster's associated account ID, if any"));
    $t->addColumn('pid', 'integer', ray('length null comment', phx::INT_BIG, TRUE, "poster's associated people record ID, if any"));
    $t->addColumn('hits', 'integer', ray('length default comment', phx::INT_BIG, 0, 'number of times details about this item have been viewed'));
    $t->addColumn('contacts', 'integer', ray('length default comment', phx::INT_BIG, 0, 'number of contacts about this item'));
    $t->addColumn('created', 'string', ray('length null comment', 11, TRUE, 'start date'));
    $t->addColumn('end', 'string', ray('length null comment', 11, TRUE, 'end date'));

    $t->addIndex(['uid']);
    $t->addIndex(['pid']);
    $t->addIndex(['type']);
    $t->addIndex(['cat']);
    $t->addIndex(['created']);
    
//    $t->addForeignKey('cat', 'service_cats', 'id', ['delete' => 'RESTRICT', 'update' => 'CASCADE']);
    $t->create();
  }
}
