Feature: Change UID
AS an Administrator
I WANT to merge two accounts or delete and account or change an account's uid
SO I can get the account data right to reflect reality.

Setup:
  Given members:
  | uid  | fullName | flags                          |*
  | .ZZA | Abe One  | member,ok,confirmed,ided       |
  | .ZZB | Bea Two  | member,ok,confirmed,ided       |
  | .ZZC | Cor Pub  | member,ok,confirmed,ided,co    |
  | .ZZD | Dee Four | member,ok,confirmed,ided,admin |
  | .ZZF | Foxco    | member,ok,confirmed,ided,co    |
  Then count "users" is 7
  # including CG and ctty
  
  Given these "admins":
  | uid  | vKeyE     | can                 |*
  | .ZZA | zot       |                     |
  | .ZZD | DEV_VKEYE | deleteAcct,seeAccts |
  Then count "admins" is 3
  # including uid=1
  Given these "txs":
  | xid | uid1 | uid2 | amt | eid | for       | type     |*
  | 1   | bank | .ZZA | 200 | 1   | from bank | %E_BANK  |
  | 2   | .ZZA | .ZZC | 10  | 3   | food      | %E_PRIME |
  Then count "txs" is 2
  Given these "txs2":
  | txid | xid | payee | amount |*
  | 4    | 1   | .ZZA  | 200    |
  Then count "txs2" is 1
  Given these "u_relations":
  | reid | main | other | permission |*
  | 5    | .ZZC | .ZZA  | manage     |
  | 6    | .ZZC | .ZZB  | buy        |
  | 7    | .ZZA | .ZZB  | none       |
  | 8    | .ZZB | .ZZD  | manage     |
  Then count "u_relations" is 4
  Given these "u_company":
  | uid  | website  |*
  | .ZZC | webc.com |
  | .ZZF | webf.com |
  Then count "u_company" is 4
  # including region and Common Good
  Given these "u_photo":
  | uid  | photo |*
  | .ZZA | Apic  |
  | .ZZB | Bpic  |
  Then count "u_photo" is 2
  Given these "tx_requests":
  | nvid | amount | payer | payee | purpose |*
  | 7    |    123 | .ZZA  | .ZZC  | invoice |
  | 8    |    456 | .ZZB  | .ZZC  | invoice |
  Then count "tx_requests" is 2
  Given these "u_shout":
  | uid  | quote |*
  | .ZZA | yesA! |
  Then count "u_shout" is 1
  Given these "tx_disputes":
  | id | xid | uid  | agentUid | reason |*
  | 9  | 2   | .ZZA | .ZZA     | bad    |
  Then count "tx_disputes" is 1
  Given these "r_notices":
  | msgid | uid  | message |*
  | 10    | .ZZA | msgA    |
  | 11    | .ZZB | msgB    |
  Then count "r_notices" is 2
  Given these "r_stakes":
  | clubid | uid  | stake | request |*
  | 2      | .ZZA | 101   | 201     |
  | 2      | .ZZB | 102   | 202     |
  Then count "r_stakes" is 2
  Given these "r_ballots":
  | id | question | voter |*
  | 11 | 1        | .ZZA  |
  | 12 | 2        | .ZZB  |
  | 13 | 3        | .ZZA  |
  Then count "r_ballots" is 3
  Given these "r_proxies":
  | person | proxy | priority |*
  | .ZZA   | .ZZD  | 1        |
  | .ZZB   | .ZZD  | 2        |
  Then count "r_proxies" is 2

Scenario: Admin views the merge page
  When member ".ZZD" visits "sadmin/merge-accounts"
  Then we show "Merge Accounts" with:
  | Merge  |
  | Into   |
  | Submit |
  
Scenario: Admin merges two personal accounts
  When member ".ZZD" submits "sadmin/merge-accounts" with:
  | from | into |*
  | .ZZA | .ZZB |
  Then members:
  | uid  | fullName | flags                          |*
  | .ZZB | Bea Two  | member,ok,confirmed,ided       |
  | .ZZC | Cor Pub  | member,ok,confirmed,ided,co    |
  | .ZZD | Dee Four | member,ok,confirmed,ided,admin |
  | .ZZF | Foxco    | member,ok,confirmed,ided,co    |
  And count "users" is 6
  And these "admins":
  | uid  | vKeyE     | can                 |*
  | .ZZB | zot       |                     |
  | .ZZD | DEV_VKEYE | deleteAcct,seeAccts |
  And count "admins" is 3
  And these "txs":
  | xid | uid1 | uid2 | amt | eid | for       | type     |*
  | 1   | bank | .ZZB | 200 | 1   | from bank | %E_BANK  |
  | 2   | .ZZB | .ZZC | 10  | 3   | food      | %E_PRIME |
  And count "txs" is 2
  And these "txs2":
  | txid | xid | payee | amount |*
  | 4    | 1   | .ZZB  | 200    |
  And count "txs2" is 1
  And these "u_relations":
  | reid | main | other | permission |*
  | 6    | .ZZC | .ZZB  | buy        |
  | 8    | .ZZB | .ZZD  | manage     |
  And count "u_relations" is 2
  And these "u_company":
  | uid  | website  |*
  | .ZZC | webc.com |
  | .ZZF | webf.com |
  And count "u_company" is 4
  And these "u_photo":
  | uid  | photo |*
  | .ZZB | Bpic  |
  And count "u_photo" is 1
  And these "tx_requests":
  | nvid | amount | payer | payee | purpose |*
  | 7    |    123 | .ZZB  | .ZZC  | invoice |
  | 8    |    456 | .ZZB  | .ZZC  | invoice |
  And count "tx_requests" is 2
  And these "u_shout":
  | uid  | quote |*
  | .ZZB | yesA! |
  And count "u_shout" is 1
  And these "tx_disputes":
  | id | xid | uid  | agentUid | reason |*
  | 9  | 2   | .ZZB | .ZZB     | bad    |
  And count "tx_disputes" is 1
  And these "r_notices":
  | msgid | uid  | message |*
  | 10    | .ZZB | msgA    |
  | 11    | .ZZB | msgB    |
  And count "r_notices" is 2
  And these "r_stakes":
  | clubid | uid  | stake | request |*
  | 2      | .ZZB | 203   | 403     |
  And count "r_stakes" is 1
  And these "r_ballots":
  | id | question | voter |*
  | 11 | 1        | .ZZB  |
  | 12 | 2        | .ZZB  |
  | 13 | 3        | .ZZB  |
  And count "r_ballots" is 3
  And these "r_proxies":
  | person | proxy | priority |*
  | .ZZB   | .ZZD  | 1        |
  | .ZZB   | .ZZD  | 2        |
  Then count "r_proxies" is 2
  # Leaving ZZB with the same proxy as their alternate is not ideal, but not terrible

Scenario: Admin merges two company accounts
  When member ".ZZD" submits "sadmin/merge-accounts" with:
  | from | into |*
  | .ZZC | .ZZF |
  Then members:
  | uid  | fullName | flags                          |*
  | .ZZA | Abe One  | member,ok,confirmed,ided       |
  | .ZZB | Bea Two  | member,ok,confirmed,ided       |
  | .ZZD | Dee Four | member,ok,confirmed,ided,admin |
  | .ZZF | Foxco    | member,ok,confirmed,ided,co    |
  And count "users" is 6
  And these "u_company":
  | uid  | website  |*
  | .ZZF | webf.com |
  And count "u_company" is 3
  And these "txs":
  | xid | uid1 | uid2 | amt | eid | for       | type     |*
  | 1   | bank | .ZZA | 200 | 1   | from bank | %E_BANK  |
  | 2   | .ZZA | .ZZF | 10  | 3   | food      | %E_PRIME |
  And count "txs" is 2
  And these "u_relations":
  | reid | main | other | permission |*
  | 5    | .ZZF | .ZZA  | manage     |
  | 6    | .ZZF | .ZZB  | buy        |
  | 7    | .ZZA | .ZZB  | none       |
  | 8    | .ZZB | .ZZD  | manage     |
  And count "u_relations" is 4
  And these "tx_requests":
  | nvid | amount | payer | payee | purpose |*
  | 7    |    123 | .ZZA  | .ZZF  | invoice |
  | 8    |    456 | .ZZB  | .ZZF  | invoice |
  And count "tx_requests" is 2

Scenario: Voting duplication prevents merging accounts
  Given these "r_ballots":
  | id | question | voter |*
  | 14 | 3        | .ZZB  |
  When member ".ZZD" submits "sadmin/merge-accounts" with:
  | from | into |*
  | .ZZA | .ZZB |
  Then we say "error": "Both of those accounts have voted on the same question, so they cannot be merged."  
  And members:
  | uid  | fullName | flags                          |*
  | .ZZA | Abe One  | member,ok,confirmed,ided       |
  | .ZZB | Bea Two  | member,ok,confirmed,ided       |
  | .ZZC | Cor Pub  | member,ok,confirmed,ided,co    |
  | .ZZD | Dee Four | member,ok,confirmed,ided,admin |
  | .ZZF | Foxco    | member,ok,confirmed,ided,co    |
  And count "users" is 7
