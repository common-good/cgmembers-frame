#!/bin/bash
# Regenerate db/startup.sql so a fresh install starts with every migration to date.
# Loads the current startup.sql into a scratch database, runs all migrations, then writes the schema of every table plus
# the data of the reference tables (KEEP). Then checks that loading the new file gives the same database (schema and KEEP
# data) and leaves no migration to run. Run from the repo root with root database access, as for importdb.sh.
# Set DBROOT for other root credentials (default "-u root"). Scratch databases are dropped unless something fails.
set -e -o pipefail; export LC_ALL=C
cd "$(dirname "$0")/.."
R=${DBROOT:--u root}; A=startup_scratch; B=startup_check; U=startup_regen; OUT=db/startup.sql
sql() { mysql $R -N "$@"; }

# Every base table must be in exactly one list. KEEP: reference data a new install needs (Drupal's boot tables, lookups,
# phinxlog so migrated steps count as done, the anonymous user 0). SKIP: caches, sessions, logs, test results, member data.
KEEP="block phinxlog post_cats r_areas r_banks r_countries r_criteria r_industries r_regions r_states r_transit
  registry registry_file system tx_cats users variable zip3 zips"
SKIP="cache cache_bootstrap cache_form cache_menu flood menu_links menu_router queue semaphore sessions test
  admins changes cu_folders cu_lists cu_members cu_spaces cu_tasks cu_times grants messages ofac people posts
  r_bad r_ballots r_boxes r_do r_events r_honors r_investments r_invites r_ips r_near r_notices r_options r_pairs
  r_proposals r_proxies r_questions r_ratings r_shares r_stakes r_stats r_tous r_usd2 r_user_industries r_votes reset_codes
  tx_bads tx_disputes_all tx_entries_all tx_hdrs_all tx_requests_all tx_rules tx_timed txs2
  u_company u_focuses u_groupies u_groups u_photo u_relations u_shout u_track
  x_company x_photo x_relations x_shout x_txs2 x_users"

fresh() { sql -e "DROP DATABASE IF EXISTS $1; CREATE DATABASE $1"; sql $1 < $2; }
tables() { sql -e "SELECT table_name FROM information_schema.tables WHERE table_schema='$1' AND table_type='BASE TABLE'" | sort; }
schema() { mysqldump $R --no-data --routines --triggers --skip-comments --skip-dump-date $1 | sed -e '/enable the sandbox mode/d' -e 's/ DEFINER=`[^`]*`@`[^`]*`//g'; }
data() { mysqldump $R --no-create-info --skip-triggers --skip-extended-insert --order-by-primary --hex-blob --skip-comments --skip-dump-date $1 $KEEP | sed '/enable the sandbox mode/d'; }
sums() { for t in $KEEP; do echo "$t $(sql -e "CHECKSUM TABLE \`$t\`" $1 | cut -f2)"; done; }

# phinx cannot use root's socket login, so it runs as a temporary user. The views it recreates name that user as definer,
# so the user must outlive the dumps.
T=$(mktemp -d); P=$(openssl rand -hex 16)
trap 'rm -rf $T; sql -e "DROP USER IF EXISTS $U@localhost"' EXIT
sql -e "DROP USER IF EXISTS $U@localhost; CREATE USER $U@localhost IDENTIFIED BY '$P'; GRANT ALL ON $A.* TO $U@localhost; GRANT ALL ON $B.* TO $U@localhost"
migrate() { # run phinx migrate on database $1 and list the migrations it ran
  cat > $T/phinx.php <<EOF
<?php return ['paths' => ['migrations' => '$PWD/db/migrations'], 'version_order' => 'creation',
  'environments' => ['default_migration_table' => 'phinxlog', 'default_environment' => 'x',
    'x' => ['adapter' => 'mysql', 'host' => 'localhost', 'name' => '$1', 'user' => '$U', 'pass' => '$P', 'charset' => 'utf8']]];
EOF
  vendor/robmorgan/phinx/bin/phinx migrate -c $T/phinx.php -e x > $T/log 2>&1 || { cat $T/log; return 1; }
  grep -E '^ == .* migrated' $T/log || true
}

echo "Loading $OUT into $A and migrating..."
fresh $A $OUT; migrate $A

all=$(tables $A); listed=$(echo $KEEP $SKIP | tr ' ' '\n' | sort)
dup=$(echo "$listed" | uniq -d); new=$(comm -23 <(echo "$all") <(echo "$listed")); gone=$(comm -13 <(echo "$all") <(echo "$listed"))
[ -n "$dup" ] && { echo "Listed twice in KEEP/SKIP: $dup"; exit 1; }
[ -n "$new" ] && { echo "Add each of these tables to KEEP or SKIP in $0:" $new; exit 1; }
[ -n "$gone" ] && echo "Warning: no longer in the database (remove from KEEP/SKIP):" $gone
nonempty=$(for t in $SKIP; do [ -n "$(sql -e "SELECT 1 FROM \`$t\` LIMIT 1" $A)" ] && echo -n "$t "; done || true)

v=$(sql -e "SELECT MAX(version) FROM phinxlog" $A)
{ echo "-- Common Good database as of migration $v. Made by db/make-startup.sh; do not edit by hand."
  schema $A; data $A; } > $OUT.new

echo "Checking: load the new file into $B..."
fresh $B $OUT.new
ran=$(migrate $B); [ -n "$ran" ] && { echo "Migrations still ran after loading the new file:"; echo "$ran"; exit 1; }
diff <(schema $A) <(schema $B) > /dev/null || { echo "Schema differs:"; diff <(schema $A) <(schema $B) | head -40; exit 1; }
diff <(sums $A) <(sums $B) || { echo "KEEP data differs (table checksums above)"; exit 1; }
for t in $SKIP; do [ -n "$(sql -e "SELECT 1 FROM \`$t\` LIMIT 1" $B)" ] && { echo "SKIP table $t has data"; exit 1; }; done || true

mv $OUT.new $OUT; sql -e "DROP DATABASE $A; DROP DATABASE $B"
echo "Wrote $OUT ($(grep -c '^INSERT' $OUT) rows, $(echo "$all" | wc -l) tables). Schema and KEEP data match the old file plus migrations."
[ -n "$nonempty" ] && echo "Data left out (SKIP tables that had rows in the old file): $nonempty"
exit 0
