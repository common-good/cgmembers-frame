Feature: Tx Detail
AS an administrator
I WANT to edit transactions
SO I can correct errors.

Setup:
  Given members:
  | uid  | fullName   | floor | acctType    | flags        | created    | risks   |*
  | .ZZA | Abe One    | -100  | personal    | ok,confirmed | %today-15m | hasBank |
  | .ZZB | Bea Two    | -200  | personal    | ok,co        | %today-15m |         |
  | .ZZC | Corner Pub | -300  | corporation | ok,co        | %today-15m |         |
  | .ZZD | Dee Four   | -400  | personal    | ok,admin     | %today-15m |         |
  And these "admins":
  | uid  | vKeyE     | can                              |*
  | .ZZD | DEV_VKEYE | seeAccts,seeTxInfo,editTx,region |
  And these "u_relations":
  | main | agent | permission |*
  | .ZZA | .ZZB  | buy        |
  | .ZZB | .ZZA  | read       |
  | .ZZC | .ZZB  | buy        |
  | .ZZC | .ZZA  | sell       |
  And these "txs2":
  | txid | payee | amount | created    | completed | deposit    |*
  |   11 |  .ZZA |   1000 | %today-13m | %now-13m  | %today-13m |
  |   12 |  .ZZB |   2000 | %today-13m | %now-13m  | %today-13m |
  |   13 |  .ZZC |   3000 | %today-13m | %now-13m  | %today-13m |
  |   14 |  .ZZA |     11 | %today-3d  |         0 | %today-13m |
  |   15 |  .ZZA |    -22 | %today-4d  | %now-4d   |          0 |
  |   16 |  .ZZA |    -33 | %today-4d  | %now-4d   |          0 |
  Then these "txs": 
  | eid | xid | created    | amount | payer | payee | purpose   |*
  |   1 |   1 | %today-13m |   1000 | bank  | .ZZA  | from bank |
  |   3 |   2 | %today-13m |   2000 | bank  | .ZZB  | from bank |
  |   4 |   3 | %today-13m |   3000 | bank  | .ZZC  | from bank |
  |   5 |   4 | %today-3d  |      0 | bank  | .ZZA  | from bank |
  |   6 |   5 | %today-4d  |    -22 | bank  | .ZZA  | to bank   |
  |   7 |   6 | %today-4d  |    -33 | bank  | .ZZA  | to bank   |
  # bug in MySql leaves a gap between eid=1 and eid=3

  Given these "txs": 
  | xid | created   | amount | payer | payee | purpose | taking | reversesXid | channel |*
  |  44 | %today-5m |     10 | .ZZB  | .ZZA  | cash E  | 0      |             |         |
  |  45 | %today-4m |   1100 | .ZZC  | .ZZA  | usd F   | 1      |             |         |
  |  46 | %today-3m |    240 | .ZZA  | .ZZB  | what G  | 0      |             |         |
  |  47 | %today-2w |     50 | .ZZB  | .ZZC  | cash P  | 0      |             |         |
  |  48 | %today-1w |    120 | .ZZA  | .ZZC  | this Q  | 1      |             |         |
  |  49 | %today-6d |    100 | .ZZA  | .ZZB  | cash V  | 0      |             |         |
  |  50 | %today-5d |    -10 | .ZZB  | .ZZA  | cash E  | 0      |          44 |         |

Scenario: an admin asks to view or edit a transaction
  When member "A:D" visits "history/transaction/xid=46"
  Then we show "Transaction #46 Detail" with:
  | Date        | %ymd-3m       |
  | Amount      | -240          |
  | For         | what G        |
  | To          | Bea Two       |
  | From        | Abe One *     |
  | Postal Addr | 1 My Street, Mytown, MA 01301 |
  | Email       | b@example.com |
  And without:
  | Category    |

Scenario: an admin asks to view or edit a community transaction
  Given these "txs": 
  | xid | created   | amount | payer | payee | purpose |*
  |  51 | %today-5d |     51 | ctty  | .ZZB  | loan    |
  When member ".ZZD" visits "history/transaction/xid=51"
  Then we show "Transaction #51 Detail" with:
  | Date        | %ymd-5d       |
  | Amount      | -51           |
  | For         | loan          |
  | To          | Bea Two       |
  | From        | Common Good Western Mass *    |
  | Postal Addr | 1 My Street, Mytown, MA 01301 |
  | Email       | b@example.com |
  | Category    |               |

Scenario: an admin sets a loan category
  Given these "txs": 
  | eid | xid | created  | amount | payer | payee | purpose | flags | type  |*
  | 501 |  51 | %now0-5d |     51 | ctty  | .ZZB  | loan    | qbok  | prime |
  And var "orig" is ray:
  | created  | cat1 | cat2 | uid1 | uid2 | flags | type | amt |*
  | %now0-5d |      |      | ctty | .ZZB | qbok  | prime | 51 |
  When member ".ZZD" submits "history/transaction/xid=51" with:
  | created | %mdY-5d |**
  | amount  | -51     |
  | toMe    |         |
  | forSame | 1       |
  | eid     | 501     |
  | cat     | CG2CG   |
  | orig    | @orig   |
  | xid     | 51      |
  Then these "txs": 
  | eid | xid | created  | amount | payer | payee | purpose | flags | cat1  | cat2  |*
  | 501 |  51 | %now0-5d |     51 | ctty  | .ZZB  | loan    |       | CG2CG | CG2CG |

Scenario: an admin changes from loan category to a different category
  Given these "txs": 
  | eid | xid | created  | amount | payer | payee | purpose | flags | type  | cat1  | cat2  |*
  | 501 |  51 | %now0-5d |     51 | ctty  | .ZZB  | loan    | qbok  | prime | CG2CG | CG2CG |
  And var "orig" is ray:
  | created  | cat1  | cat2  | uid1 | uid2 | flags | type | amt |*
  | %now0-5d | CG2CG | CG2CG | ctty | .ZZB | qbok  | prime | 51 |
  When member ".ZZD" submits "history/transaction/xid=51" with:
  | created | %mdY-5d |**
  | amount  | -51     |
  | toMe    |         |
  | forSame | 1       |
  | eid     | 501     |
  | cat     | CG-GRANT-ORG |
  | orig    | @orig   |
  | xid     | 51      |
  Then these "txs": 
  | eid | xid | created  | amount | payer | payee | purpose | flags | cat1         | cat2 |*
  | 501 |  51 | %now0-5d |     51 | ctty  | .ZZB  | loan    |       | CG-GRANT-ORG |      |

Scenario: an admin changes a transaction amount and date
  Given these "txs": 
  | eid | xid | created | amount | payer | payee | purpose | flags | type  |*
  | 501 |  51 | %now-5d |     51 | .ZZA  | .ZZB  | stuff   | gift  | prime |
  And var "orig" is ray:
  | created | cat1  | cat2  | uid1 | uid2 | flags | type  | amt |*
  | %now-5d |       |       | .ZZA | .ZZB | gift  | prime | 51 |
  When member ".ZZD" submits "history/transaction/xid=51" with:
  | created | %mdY-3d      |**
  | amount  | 52           |
  | toMe    | 1            |
  | description | new!     |
  | forSame |              |
  | eid     | 501          |
  | cat     | CG-GRANT-ORG |
  | orig    | @orig        |
  | xid     | 51           |
  Then these "txs": 
  | eid | xid | created  | amount | payer | payee | for1  | for2 | flags        | cat1 | cat2         |*
  | 501 |  51 | %now0-3d |     52 | .ZZA  | .ZZB  | stuff | new! | gift,changed |      | CG-GRANT-ORG |
  And these "changes":
  | id | table | rid | field   | oldValue | newValue | changedBy |*
  | 1  | txs   | 51  | amt     | 51       | 52       | .ZZD      |
  | 2  | txs   | 51  | created | %now-5d  | %now0-3d | .ZZD      |

Scenario: an admin changes a bank transaction amount and date
  Given var "orig" is ray:
  | created   | cat1  | cat2  | uid1 | uid2 | flags | type  | amt |*
  | %today-4d |       |       | bank | .ZZA |       | bank  | -22 |
  When member ".ZZD" submits "history/transaction/xid=2" with:
  | created | %mdY-3d      |**
  | amount  | 23           |
  | toMe    |              |
  | description | new!     |
  | forSame |              |
  | eid     | 6            |
  | cat     | CG-GRANT-ORG |
  | orig    | @orig        |
  | xid     | 5            |
  Then these "txs": 
  | eid | xid | created  | amount | payer | payee | for2    | for1 | flags   | cat1 | cat2     |*
  | 6   | 5   | %now0-3d |    -23 | bank  | .ZZA  | to bank | new! | changed | CG-GRANT-ORG |  |
  And these "txs2":
  | txid | payee | amount | created  | completed | deposit |*
  |   15 |  .ZZA |    -23 | %now0-3d | %now-4d   | 0       |
  And these "changes":
  | id | table | rid | field   | oldValue  | newValue | changedBy |*
  | 1  | txs   | 5   | amt     | -22       | -23      | .ZZD      |
  | 2  | txs   | 5   | created | %today-4d | %now0-3d | .ZZD      |

Scenario: an admin changes a deposited bank transaction amount
  Given var "orig" is ray:
  | created   | cat1  | cat2  | uid1 | uid2 | flags | type  | amt  |*
  | %now0-13m |       |       | bank | .ZZB |       | bank  | 2000 |
  When member ".ZZD" submits "history/transaction/xid=2" with:
  | created | %mdY-3d      |**
  | amount  | 2001         |
  | toMe    | 1            |
  | description | new!     |
  | forSame |              |
  | eid     | 3            |
  | cat     | CG-GRANT-ORG |
  | orig    | @orig        |
  | xid     | 2            |
  Then we say "error": "deposit amt unchangeable"
