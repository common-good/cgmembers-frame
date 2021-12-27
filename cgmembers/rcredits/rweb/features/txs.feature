Feature: Transactions
AS a member
I WANT to review my transactions
SO I can see what happened, accept or refuse offers, adjust descriptions, and correct errors.

Setup:
  Given members:
  | uid  | fullName   | floor | acctType    | flags                | created    | risks   |*
  | .ZZA | Abe One    | -100  | personal    | ok,roundup,confirmed,bankOk | %today-15m | hasBank |
  | .ZZB | Bea Two    | -200  | personal    | ok,co                | %today-15m |         |
  | .ZZC | Corner Pub | -300  | corporation | ok,co                | %today-15m |         |
  And these "u_relations":
  | main | agent | permission |*
  | .ZZA | .ZZB  | buy        |
  | .ZZB | .ZZA  | read       |
  | .ZZC | .ZZB  | buy        |
  | .ZZC | .ZZA  | sell       |
  And these "txs2":
  | txid | payee | amount | created    | completed  | deposit    |*
  |   11 |  .ZZA |   1000 | %today-13m | %today-13m | %today-13m |
  |   12 |  .ZZB |   2000 | %today-13m | %today-13m | %today-13m |
  |   13 |  .ZZC |   3000 | %today-13m | %today-13m | %today-13m |
  |   14 |  .ZZA |     11 | %today-3d  |         0  | %today-13m |
  |   15 |  .ZZA |    -22 | %today-4d  | %today-4d  |          0 |
  |   16 |  .ZZA |    -33 | %today-4d  | %today-4d  |          0 |
  Then these "txs": 
  | xid | created    | amount | payer   | payee | purpose   |*
  |   1 | %today-13m |   1000 | bank-in | .ZZA  | from bank |
  |   2 | %today-13m |   2000 | bank-in | .ZZB  | from bank |
  |   3 | %today-13m |   3000 | bank-in | .ZZC  | from bank |
  |   4 | %today-3d  |      0 | bank-in | .ZZA  | from bank |
  |   5 | %today-4d  |    -22 | bank-out| .ZZA  | to bank   |
  |   6 | %today-4d  |    -33 | bank-out| .ZZA  | to bank   |
  And balances:
  | uid  | balance |*
  | .ZZA |     945 |
  | .ZZB |    2000 |
  | .ZZC |    3000 |
  Given these "txs": 
  | xid | created   | amount | payer | payee | purpose | taking | reversesXid |*
  |  44 | %today-5m |     10 | .ZZB  | .ZZA  | cash E  | 0      |             |
  |  45 | %today-4m |   1100 | .ZZC  | .ZZA  | usd F   | 1      |             |
  |  46 | %today-3m |    240 | .ZZA  | .ZZB  | what G  | 0      |             |
  |  47 | %today-2w |     50 | .ZZB  | .ZZC  | cash P  | 0      |             |
  |  48 | %today-1w |    120 | .ZZA  | .ZZC  | this Q  | 1      |             |
  |  49 | %today-6d |    100 | .ZZA  | .ZZB  | cash V  | 0      |             |
  |  50 | %today-5d |    -10 | .ZZB  | .ZZA  | cash E  | 0      |          44 |
  Then balances:
  | uid  | balance |*
  | .ZZA |    1585 |
  | .ZZB |    2290 |
  | .ZZC |    2070 |

Scenario: A member looks at transactions for the past year
  Given members have:
  | uid  | fullName |*
  | ctty | ZZrCred  |
  When member ".ZZA" visits page "history/transactions/period=365"
  Then we show "Transaction History" with:
  | Start        |   | 1,000.00 | %dmy-12m |
  | From Bank    | + |     0.00 | + 11.00 Pending |
  | To Bank      | - |    55.00 |          |
  | Received     | + | 1,100.00 |          |
  | Out          | - |   460.00 |          |
  | End          |   | 1,585.00 | %dmy     |
  And with:
  | Tx# | Date    | Name          | Purpose                  | Amount   |  Balance | Action |
  |  6  | %mdy-4d | --            | to bank                  |   -33.00 | 1,585.00 |        |
  |  5  | %mdy-4d | --            | to bank                  |   -22.00 | 1,618.00 |        |
  | 50  | %mdy-5d | Bea Two       | (reverses tx #44)           |   -10.00 | 1,640.00 |        |
  | 49  | %mdy-6d | Bea Two       | cash V                   |  -100.00 | 1,650.00 |        |
  | 48  | %mdy-1w | Corner Pub    | this Q                   |  -120.00 | 1,750.00 |        |
  | 46  | %mdy-3m | Bea Two       | what G                   |  -240.00 | 1,870.00 |        |
  | 45  | %mdy-4m | Corner Pub    | usd F                    | 1,100.00 | 2,110.00 |        |
  | 44  | %mdy-5m | Bea Two       | cash E (reversed by #50) |    10.00 | 1,010.00 |        |
  And without:
  | rebate  |
  | bonus   |

Scenario: A member looks at transactions for the past few days
  When member ".ZZA" visits page "history/transactions/period=15"
  Then we show "Transaction History" with:
  | Start        |   | 1,870.00 | %dmy-15d |
  | From Bank    | + |     0.00 | + 11.00 Pending |
  | To Bank      | - |    55.00 |          |
  | Received     | + |   -10.00 |          |
  | Out          | - |   220.00 |          |
  | End          |   | 1,585.00 | %dmy     |
  And with:
  | Tx# | Date    | Name          | Purpose               | Amount  |  Balance |
  |  6  | %mdy-4d | --            | to bank               |  -33.00 | 1,585.00 |
  |  5  | %mdy-4d | --            | to bank               |  -22.00 | 1,618.00 |
  | 50  | %mdy-5d | Bea Two       | (reverses tx #44)        |  -10.00 | 1,640.00 |
  | 49  | %mdy-6d | Bea Two       | cash V                | -100.00 | 1,650.00 |
  | 48  | %mdy-1w | Corner Pub    | this Q                | -120.00 | 1,750.00 |
  And without:
  | pie N    |
  | whatever |
  | usd F    |
  | signup   |
  | rebate   |
  | bonus    |

Scenario: A member looks at transactions with roundups
  Given these "txs":
  | xid | amount | payer | payee            | purpose          | taking | goods      | channel | type     |*
  |  41 |  49.95 | .ZZA  | .ZZC             | sundries         | 1      | %FOR_GOODS | %TX_APP | prime    |
  |  41 |   0.05 | .ZZA  | %UID_ROUNDUPS | roundup donation | 0      | %FOR_GOODS | %TX_APP | aux      |
  Then balances:
  | uid  | balance |*
  | .ZZA | 1535.00 |
  When member ".ZZA" visits page "history/transactions/period=15"
  Then we show "Transaction History" with:
  | Start        |   | 1,870.00 | %dmy-15d |
  | From Bank    | + |     0.00 | + 11.00 Pending |
  | To Bank      | - |    55.00 |          |
  | Received     | + |   -10.00 |          |
  | Out          | - |   270.00 |          |
  | End          |   | 1,535.00 | %dmy     |
  And with:
  | Tx# | Date    | Name            | Purpose                  | Amount  |  Balance |~do |
  |     | %mdy    | Corner Pub      | sundries                 |  -49.95 | 1,535.00 |    |
  |     |         | %PROJECT Region | roundup donation         |   -0.05 |          |    | 
  |  6  | %mdy-4d | --              | to bank                  |  -33.00 | 1,585.00 |    |
  |  5  | %mdy-4d | --              | to bank                  |  -22.00 | 1,618.00 |    |
  | 50  | %mdy-5d | Bea Two         | (reverses tx #44)           |  -10.00 | 1,640.00 |    |
  | 49  | %mdy-6d | Bea Two         | cash V                   | -100.00 | 1,650.00 |    |
  | 48  | %mdy-1w | Corner Pub      | this Q                   | -120.00 | 1,750.00 |    |

Scenario: Admin reverses a bank transfer
  Given members:
  | uid  | fullName  | flags              |*
  | .ZZD | Dee Admin | ok,confirmed,admin |
  And these "admins":
  | uid  | vKeyE     | can           |*
  | .ZZD | DEV_VKEYE | reverseBankTx |
  When member ".ZZD" scans admin card "%DEV_VKEYPW"
  When member "A:D" visits page "history/transactions/period=5"
  And member "A:D" clicks X on transaction 1
  Then these "txs2":
  | txid | payee | amount | created  | completed  | deposit | xid |*
  |  -11 |  .ZZA |  -1000 | %now-13m | %now-13m   | %now    |  51 |
  And these "txs":
  | xid | created | amt2  | uid1    | uid2 | description | type |*
  |  51 | %now    | -1000 | bank-in | .ZZA | to bank     | bank |
