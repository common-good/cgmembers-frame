Feature: Categories (cats)
AS an administrator
I WANT to view and edit Rules and Timed records
SO I can tweak settings at a member's request (or to correct errors)

Setup:
  Given members:
  | uid  | fullName | flags              |*
  | .ZZA | Abe One  | ok,confirmed,admin |
  | .ZZB | Bea Two  | ok,confirmed,debt  |
  | .ZZC | Our Pub  | ok,confirmed,co    |
  | .ZZF | Food Fund | ok,confirmed,co  |
  | .ZZG | Glo Co    | ok,confirmed,co  |
  | .ZZH | Hip Co    | ok,confirmed,co  |
  And these "admins":
  | uid  | vKeyE     | can                                  |*
  | .ZZA | DEV_VKEYE | v,panel,editTx,code,makeRules,region |
  And these "people":
  | pid | fullName |*
  | 101 | Yoyo Yot |
  | 102 | Zeta Zot |
  And member ".ZZC" is sponsored
  And these "tx_rules":
  | id        | 1            |**
  | action    | %ACT_SURTX   |
  | from      | %MATCH_PAYEE |
  | to        | cgf          |
  | amount    | 0            |
  | portion   | .05          |
  | purpose   | sponsor      |
  | payerType | %REF_ANYBODY |
  | payer     |              |
  | payeeType | %REF_ACCOUNT |
  | payee     | .ZZC         |
  | minimum   | 0            |
  | useMax    |              |
  | amtMax    |              |
  | start     | %now         |
  | end       |              |
  | code      |              |
  | template  |              |
  And these "tx_timed":
  | id        | 1            | 2            | 3            | 4             | 5            |**
  | action    | pay          | pay          | surtx        | charge        | surtx        |
  | from      | .ZZB         | .ZZB         | %MATCH_PAYEE | .ZZA          | .ZZF         |
  | to        | .ZZC         | .ZZG         | %MATCH_PAYER | .ZZC          | %MATCH_PAYER |
  | amount    | 100          | 2            | 0            | 123           | 20           |
  | portion   | 0            | 0            | .03          | 0             | 0            |
  | purpose   | payment      | donation     | discount     | invoice       | food help    |
  | payerType | anybody      | anybody      | anybody      | account       | group        |
  | payer     | %NUL         | %NUL         | %NUL         | %NUL          | 1            |
  | payeeType | anybody      | anybody      | account      | anybody       | industry     |
  | payee     | %NUL         | %NUL         | .ZZC         | %NUL          | 2            |
  | minimum   | 0            | 0            | 0            | 0             | 0            |
  | useMax    | %NUL         | %NUL         | 2            | %NUL          | %NUL         |
  | amtMax    | 30           | 2            | %NUL         | 123           | 20           |
  | start     | %now-15d     | %now         | %now         | %now-3m       | %now         |
  | end       | %NUL         | %NUL         | %NUL         | %NUL          | %NUL         |
  | period    | week         | month        | quarter      | month         | month        |
  | periods   | 2            | 1            | 1            | 1             | 1            |
  | duration  | once         | once         | month        | once          | forever      |
  | durations | 1            | 1            | 1            | 1             | 1            |

Scenario: admin visits the Rules page
  When member ".ZZA" visits page "sadmin/rules"
  Then we show "Tx Rules" with:
  |id|action|from        |to   |amount|portion|purpose|payerType   |payer|payeeType|payee  |minimum|useMax|amtMax|start|end|code|
  |1 |surtx |%MATCH_PAYEE|%CGID|0     |.05    |sponsor|%REF_ANYBODY|   |%REF_ACCOUNT|ourpub|0      |      |      |%dmqy|   |    |
  # ID fields don't get interpreted here because these are displayed values, not actual field values

Scenario: admin chooses a rule to edit
  When member ".ZZA" visits page "sadmin/rules/id=1"
  Then we show "Tx Rules" with:
  | When Payer: ||
  | Payer Type: ||
  | Pays Payee: | %ZZC |
  | Payee Type: | %MATCH_PAYEE |
  | To:         | %CGID |
  | Action:     ||
  | Amount:     | 0.00 |
  | Portion:    | 0.050000 |
  | Purpose:    | sponsor |
  | Minimum:    | 0.00 |
  | Use Max:    ||
  | Extra Max:  ||
  | Start:      | %mdY |
  | End:        ||
  | Submit      ||
  # we need to check what is checked here also, in the radio buttons
 
Scenario: admin visits the Timed page
  When member ".ZZA" visits page "sadmin/timed"
  Then we show "Tx Timed" with:
  |id|action|from|to          |amount|portion |purpose  |payerType |payer|payeeType|payee|minimum|useMax|amtMax|start|end|period|periods|duration|durations|stripeId|
  |5 |surtx |%ZZF|%MATCH_PAYER|20.00 |0.000000|food help|%REF_GROUP|1|%REF_INDUSTRY|2    |0.00   |      |20.00 |%dmqy|   |month |1       |forever |1        |        |

Scenario: admin chooses a Timed record to edit
  When member ".ZZA" visits page "sadmin/timed/id=5"
  Then we show "Tx Timed" with:
  | When Payer: | 1 |
  | Payer Type: | |
  | Pays Payee: | 2 |
  | Payee Type: | |
  | Then From:  | %ZZF |
  | To:         | %MATCH_PAYER |
  | Action:     | surtx |
  | Amount:     | 20.00 |
  | Portion:    | 0.000000 |
  | Purpose:    | food help |
  | Minimum:    | 0.00 |
  | Use Max:    ||
  | Extra Max:  | 20.00 |
  | Start:      | %mdY |
  | End:        ||
  | Flags:      | 0 |
  | Period:     | month |
  | Periods:    | 1 |
  | Duration:   | |
  | Durations:  | 1 |
  | Submit      ||
  # we need to check what is checked here also, in the radio buttons
 