Feature: Deletion
AS an Administrator
I WANT to have a record of what's been deleted from crucial tables
SO I can reverse those deletions if necessary, or look up historical data.

Setup:
  Given members:
  | uid  | fullName | flags           |*
  | .ZZA | Abe One  | ok,confirmed    |
  | .ZZB | Bea Two  | ok,confirmed    |
  | .ZZC | Cor Pub  | ok,confirmed,co |
  Then count "users" is 5
  # including CG and ctty
  
  Given these "txs":
  | xid | uid1         | uid2 | amt | eid | for       | type     |*
  | 1   | %UID_BANK_IN | .ZZA | 200 | 1   | from bank | %E_BANK  |
  | 2   | .ZZA         | .ZZC | 10  | 3   | food      | %E_PRIME |
  And these "txs2":
  | txid | xid | payee | amount |*
  | 4    | 1   | .ZZA  | 200    |
  And these "u_relations":
  | reid | main | other | permission |*
  | 5    | .ZZC | .ZZA  | manage     |
  | 6    | .ZZC | .ZZB  | buy        |
  And these "u_company":
  | uid  | website |*
  | .ZZC | zot.com |
  And these "u_photo":
  | uid  | photo |*
  | .ZZA | Apic  |
  | .ZZB | Bpic  |
  And these "tx_requests":
  | nvid | amount | payer | payee | purpose |*
  | 7    |    123 | .ZZA  | .ZZC  | invoice |
  | 8    |    456 | .ZZB  | .ZZC  | invoice |
  And these "u_shout":
  | uid  | quote |*
  | .ZZA | yesA! |
  | .ZZB | yesB! |
  And these "tx_disputes":
  | id | xid | uid  | agentUid | reason |*
  | 9  | 2   | .ZZA | .ZZA     | bad    |
  And these "r_notices":
  | msgid | uid  | message |*
  | 10    | .ZZA | msgA    |
  | 11    | .ZZB | msgB    |

# tx_hdrs tx_entries tx_disputes tx_requests
Scenario: A deleted-by-view record is deleted
  When we delete table "txs" record "xid:2"
  Then these "txs":
  | xid | uid1         | uid2 | amt | eid | for       | type    |*
  | 1   | %UID_BANK_IN | .ZZA | 200 | 1   | from bank | %E_BANK |
  And count "txs" is 1
  And these "tx_hdrs_all":
  | deleted | xid |*
  | %now    | 2   |
  And count "tx_hdrs" is 1
  And these "tx_entries_all":
  | deleted | id | amount |*
  | %now    | -3 |    -10 |
  | %now    |  3 |     10 |
  And count "tx_entries" is 2

  When we delete table "tx_disputes" record "id:9"
  Then these "tx_disputes_all":
  | deleted | xid |*
  | %now    | 2   |
  And count "tx_disputes" is 0
  
  When we delete table "tx_requests" record "nvid:8"
  Then these "tx_requests_all":
  | deleted | nvid | amount | payer | payee | purpose |*
  | %now    | 8    |    456 | .ZZB  | .ZZC  | invoice |
  And count "tx_requests" is 1

# users u_shout u_company u_photo u_relations txs2
Scenario: A track-deleted record is deleted
  When we delete table "u_shout" record "uid:.ZZB"
  Then these "x_shout":
  | deleted | uid  | quote |*
  | %now    | .ZZB | yesB! |
  And count "u_shout" is 1

  When we delete table "u_company" record "uid:.ZZC"
  Then these "x_company":
  | deleted | uid  | website |*
  | %now    | .ZZC | zot.com |
  And count "u_company" is 2
  # including CG and ctty

  When we delete table "u_photo" record "uid:.ZZB"
  Then these "x_photo":
  | deleted | uid  | photo |*
  | %now    | .ZZB | Bpic  |
  And count "x_photo" is 1
  And count "u_photo" is 1

  When we delete table "u_relations" record "reid:6"
  Then these "x_relations":
  | deleted | reid |*
  | %now    | 6    |
  And count "u_relations" is 1

  When we delete table "txs2" record "txid:4"
  Then these "x_txs2":
  | deleted | txid |*
  | %now    | 4    |
  And count "x_txs2" is 1
  And these "txs":
  | xid | uid1         | uid2 | amt | eid | for       |*
  | 2   | .ZZA         | .ZZC | 10  | 3   | food      |
  And count "txs" is 1
  And these "tx_hdrs_all":
  | deleted | xid |*
  | %now    | 1   |
  And count "tx_hdrs" is 1
  And these "tx_entries_all":
  | deleted | id | amount |*
  | %now    | -1 | -200   |
  | %now    | 1  | 200    |
  And count "tx_entries" is 2

Scenario: A member record is deleted
  When we delete table "users" record "uid:.ZZB"
  Then these "x_users":
  | deleted | uid  | fullName |*
  | %now    | .ZZB | Bea Two  |
  And count "x_users" is 1
  And count "users" is 4

  And these "x_photo":
  | deleted | uid  | photo |*
  | %now    | .ZZB | Bpic  |
  And count "x_photo" is 1
  And count "u_photo" is 1
  
  And these "x_shout":
  | deleted | uid  | quote |*
  | %now    | .ZZB | yesB! |
  And count "u_shout" is 1

  And these "u_relations":
  | reid | main | other | permission |*
  | 5    | .ZZC | .ZZA  | manage     |
  And count "u_relations" is 1
  
  And these "tx_requests":
  | nvid | amount | payer | payee | purpose |*
  | 7    |    123 | .ZZA  | .ZZC  | invoice |
  And count "tx_requests" is 1
  
  And these "r_notices":
  | msgid | uid  | message |*
  | 10    | .ZZA | msgA    |
  And count "r_notices" is 1

Scenario: A record is deleted from an untracked table
  Given these "r_invites":
  | id  | inviter | message |*
  | 12  | .ZZB    | do it!  |
  When we delete table "r_invites" record "id:12"
  Then count "r_invites" is 0
