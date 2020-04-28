<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class PerYearFunction extends AbstractMigration {
  const PERIODS = 'once day week month quarter year forever'; // periods and durations

  public function up() {
    $periodList = "ENUM('" . join("','", ray(self::PERIODS)) . "')";

    $this->execute('DROP FUNCTION IF EXISTS perYear');

    $this->execute(<<< X
      CREATE FUNCTION perYear(period $periodList, periods INT(11))
      RETURNS TINYINT
      DETERMINISTIC
      RETURN (CASE period
        WHEN 'forever' THEN 0
        WHEN 'year' THEN 1
        WHEN 'quarter' THEN 4
        WHEN 'month' THEN 12
        WHEN 'week' THEN 365.25/7
        WHEN 'day' THEN 365.25
        WHEN 'once' THEN 0
        ELSE -1
      END) / periods;
      END
X
    );
  }
  
  public function down() {
    $this->execute('DROP FUNCTION perYear');
  }
}
