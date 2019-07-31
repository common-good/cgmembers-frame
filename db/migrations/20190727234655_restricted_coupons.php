<?php

use Phinx\Migration\AbstractMigration;

class RestrictedCoupons extends AbstractMigration {

  public function up() {
    $this->execute('UPDATE r_coupons SET flags=1024 WHERE flags=1');
    $this->execute("UPDATE variable SET name='reconciledAsOf' WHERE name='\$reconciledAsOf'");
    $this->execute("UPDATE variable SET name='last_cron_end' WHERE name='r_last_cron'");

    foreach (explode(' ', 'install_ node_ r_ sms_ site_ theme_ update_ user_') as $k) {
      $this->execute("DELETE FROM variable WHERE name LIKE '$k%'");
    }
    $names = explode(' ', 'admin_theme anonymous clean_url cron_last css_js_query_string ctools_last_cron cttyPaidEver daily date_default_timezone default_nodes_main drupal_http_request_fails duplicate_email_message email__active_tab file_temporary_path filter_fallback_format forgot_password_message last_daily_cron mail_system maintenance_mode maintenance_mode_message path_alias_whitelist rcredits_preserve signupData totals');
    $names = "'" . join("', '", $names) . "'";
    $this->execute("DELETE FROM variable WHERE name IN ($names)"); // no need to reverse; these are long unused
  }

  public function down() {
    $this->execute('UPDATE r_coupons SET flags=1 WHERE flags=1024');
    $this->execute("UPDATE variable SET name='\$reconciledAsOf' WHERE name='reconciledAsOf'");
    $this->execute("UPDATE variable SET name='r_last_cron' WHERE name='last_cron_end'");
  }
}
