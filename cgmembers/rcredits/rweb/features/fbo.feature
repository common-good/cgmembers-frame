Feature: FBO
AS a sponsored member company or administrator
I WANT to track payments to and from non-members
SO I can accept donations and make payments for a fiscally-sponsored organization.

Setup:
  Given members:
  | uid  | fullName | phone        | address | city  | state  | zip | country  | postalAddr | floor | flags              | coFlags   | emailCode |*
  | .ZZA | Abe One  | +13013013001 | 1 A St. | Atown | Alaska | 01000 | US     | 1 A, A, AK |  -250 | ok,confirmed,debt  |           |           |
  | .ZZB | Bea Two  |              | 2 B St. | Btown | Utah   | 02000 | US     | 2 B, B, UT |  -250 | ok,confirmed,debt  |           |           |
  | .ZZC | Our Pub  | +13333333333 | 3 C St. | Ctown | Cher   |       | France | 3 C, C, FR |     0 | ok,confirmed,co    | sponsored | Cc3       |
  And these "u_relations":
  | main | agent | permission |*
  | .ZZA | .ZZB  | buy        |
  | .ZZB | .ZZA  | read       |
  | .ZZC | .ZZB  | buy        |
  | .ZZC | .ZZA  | manage     |
  And these "tx_rules":
  | id        | 1            |**
  | payer     |              |
  | payerType | %REF_ANYBODY |
  | payee     | .ZZC         |
  | payeeType | %REF_ACCOUNT |
  | from      | %MATCH_PAYEE |
  | to        | cgf          |
  | action    | %ACT_SURTX   |
  | amount    | 0            |
  | portion   | .05          |
  | purpose   | sponsor      |
  | minimum   | 0            |
  | useMax    |              |
  | amtMax    |              |
  | template  |              |
  | start     | %now         |
  | end       |              |
  | code      |              |
  Then balances:
  | uid  | balance |*
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |       0 |

Scenario: A non-member donates to a sponsored member
  When member "C:A" visits "tx/charge"
  Then we show "Charge" with:
  | Member      | Non-member |
  | Full Name   | |
  | Postal Addr | |
  | City        | |
  | State       | |
  | Zip         | |
  | Amount      | |
  | For         | |
  | Category    | |
  
  Given members have:
  | uid  | flags    |*
  | .ZZA | ok,admin |
  When member "C:A" submits "tx/charge" with:
  | op     | fbo | fullName | email | address | city | state | zip   | amount | purpose | cat | comment |*
  | charge | 1   | Dee Forn | d@    | 4 Fr St | Fton | MA    | 01004 | 100    | grant   |   2 |         |
  Then we scrip "tx" with subs:
  | field | question            | selfErr | payDesc | chargeDesc |*
  | who   | %_%amount to %name? | self-tx | Pay     | Charge     |
  # choice between Pay and Charge gets set in JS
  And we say "status": "info saved"
  And these "txs":
  | eid | xid | payer      | payee | amount | purpose | cat | type     |*
  |   1 | 1   | %UID_OUTER | .ZZC  | 100    | grant   | 2   | %E_OUTER |
  |   3 | 1   | .ZZC       | cgf   | 5      | sponsor | 2   | %E_AUX   |
  And these "txs2":
  | xid | payee | amount | completed | deposit | pid |*
  | 1   | .ZZC  | 100    | %now      | %now    | 1   |
  And these "people":
  | pid | fullName | address | city | state | zip   |*
  | 1   | Dee Forn | 4 Fr St | Fton | MA    | 01004 |
  And balances:
  | uid  | balance |*
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |      95 |
  | cgf  |       5 |
  And we email "fbo-thanks" to member "d@" with subs:
  | fullName     | Dee Forn        |**
  | date         | %mdY            |
  | coName       | Our Pub         |
  | coPostalAddr | 3 C, C, FR      |
  | coPhone      | +1 333 333 3333 |
  | gift         | $100            |
  And we email "fbo-report" to member "c@" with subs:
  | gift         | $100                 |**
  | date         | %mdY                 |
  | donor        | Dee Forn             |
  | donorAddress | 4 Fr St, Fton, MA 01004 |
  | donorPhone   |                      |
  | donorEmail   | d@example.com        |
  | fullName     | Our Pub              |
  | note         |                      |

Scenario: A sponsored member pays a nonmember
  When member "C:A" visits "tx/pay"
  Then we show "Pay" with:
  | Member      | Non-member |
  | Full Name   | |
  | Postal Addr | |
  | City        | |
  | State       | |
  | Zip         | |
  | Amount      | |
  | For         | |
  | Category    | |
  
  When member "C:A" submits "tx/pay" with:
  | op  | fbo | fullName | address | city | state | zip   | amount | purpose  | cat |*
  | pay | 1   | Dee Forn | 4 Fr St | Fton | MA    | 01004 | 100    | printing |   3 |
  Then we scrip "tx" with subs:
  | field | question            | selfErr | payDesc | chargeDesc |*
  | who   | %_%amount to %name? | self-tx | Pay     | Charge     |
  # choice between Pay and Charge gets set in JS
  And we say "status": "info saved"
  And these "txs":
  | xid | payer      | payee | amount | purpose  | cat | type     |*
  | 1   | %UID_OUTER | .ZZC  | -100   | printing | 3   | %E_OUTER |
  And these "txs2":
  | xid | payee | amount | completed | deposit | pid |*
  | 1   | .ZZC  | -100   | %now      |       0 | 1   |
  And these "people":
  | pid | fullName | address | city | state | zip   |*
  | 1   | Dee Forn | 4 Fr St | Fton | MA    | 01004 |
  And balances:
  | uid  | balance |*
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |    -100 |

Scenario: A sponsored member charges a member
  When member "C:A" submits "tx/charge" with:
  | op     | fbo | who  | amount | purpose | cat |*
  | charge |   1 | .ZZB |    100 | grant   |   2 |
  Then we scrip "tx" with subs:
  | field | question            | selfErr | payDesc | chargeDesc |*
  | who   | %_%amount to %name? | self-tx | Pay     | Charge     |
  Then we say "status": "report tx|balance unchanged" with subs:
  | did     | otherName | amount |*
  | charged | Bea Two   | $100   |
  And we message "invoiced you" to member ".ZZB" with subs:
  | otherName | amount | purpose |*
  | Our Pub   | $100   | grant   |
  And these "tx_requests":
  | nvid | created | status      | amount | payer | payee | for   | cat |*
  |    1 | %today  | %TX_PENDING |    100 | .ZZB  | .ZZC  | grant |   2 |
  And balances:
  | uid  | balance |*
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |       0 |

  When member ".ZZB" confirms form "handle-invoice/nvid=1&code=TESTDOCODE" with values:
  | op   | ret | nvid | payAmount | payer | payee | purpose | created |*
  | pay  |     |    1 |       100 | .ZZB  | .ZZC  | grant   | %today  |
  Then these "txs":
  | xid | created | amount | payer | payee | purpose | taking | relType | rel | cat |*
  |   1 | %today  |    100 | .ZZB | .ZZC   | grant   | 0      | I       | 1   |   2 |

Scenario: A sponsored member views their transaction history
  Given these "txs":
  | xid | payer      | payee | amount | purpose  | cat | type     | payeeAgent |*
  | 1   | %UID_OUTER | .ZZC  | 100    | grant    | 2   | %E_OUTER | .ZZB       |
  | 2   | %UID_OUTER | .ZZC  | -200   | printing | 21  | %E_OUTER | .ZZB       |
  And these "txs2":
  | xid | payee | amount | completed | deposit | pid |*
  | 1   | .ZZC  | 100    | %now      |       0 | 4   |
  | 2   | .ZZC  | -100   | %now      |       0 | 5   |
  And these "people":
  | pid | fullName | address | city | state | zip   |*
  | 4   | Dee Forn | 4 D St  | Dton | MA    | 01004 |
  | 5   | Eva Fivn | 5 E St  | Eton | CA    | 01005 |
  When member "C:A" visits "history/transactions"
  Then we show "Transaction History" with:
  | Tx | Date | Name                  | Purpose  | Amount  | Balance |
  | 2  | %mdy | Eva Fivn (non-member) | printing | -200.00 | -100.00 |
  | 1  | %mdy | Dee Forn (non-member) | grant    |  100.00 |  100.00 |

  When member "C:A" visits "history/transaction/xid=1"
  Then we show "Transaction #1 Detail" with:
  | Date        | %mdY |
  | Amount      | $100 |
  | From        | Dee Forn (non-member) |
  | Postal Addr | 4 D St, Dton, MA 01004 |
  | For         | grant |
  | Category    | Government grants |
  | Our Agent   | Bea Two |
  | Channel     | Web |

Scenario: A non-member donates to a sponsored organization by credit card
  Given a button code for:
  | account | secret |*
  | .ZZC    | Cc3    |
  When member "?" visits "community/donate/code=TESTCODE"
  Then we show "Donate to Our Pub" with:
  | Donation    |
  | Name        |
  | Phone       |
  | Email       |
  | Country     |
  | Postal Code |
  | Pay By      |
  | Donate      |

  Given next captcha is "37"
  And var "CODE" encrypts:
  | pid | amount | period | coId   |*
  | 1   | 123.00 | once   | NEWZZC |
  When member "?" completes "community/donate/code=TESTCODE" with:
  | amount | fullName | phone        | email | zip   | payHow | comment  | cq | ca |*
  |    123 | Zee Zot  | 262-626-2626 | z@    | 01301 |      1 | awesome! | 37 | 74 |
  Then these "people":
  | pid | fullName | phone        | email | zip   | state |*
  | 1   | Zee Zot  | +12626262626 | z@    | 01301 | MA    |

  And we redirect to "https://www.paypal.com/donate"
  And return URL "/community/donate/op=done&code=CODE"
  
  When member "?" visits "community/donate/op=done&code=CODE"
  Then these "txs2":
  | xid | payee | amount | completed | deposit | pid |*
  | 1   | .ZZC  | 123    | %now      |    %now | 1   |
  And these "txs":
  | eid | xid | payer      | payee | amount | purpose    | type       |*
  | 1   | 1   | %UID_OUTER | .ZZC  | 123    | donation   | %E_OUTER   |
  | 3   | 1   | .ZZC       | cgf   | 6.15   | sponsor    | %E_AUX     |
  | 4   | 1   | .ZZC       | cgf   | 3.69   | fbo cc fee | %E_USD_FEE |
  And we email "fbo-thanks" to member "z@" with subs:
  | fullName     | Zee Zot         |**
  | date         | %mdY            |
  | coName       | Our Pub         |
  | coPostalAddr | 3 C, C, FR      |
  | coPhone      | +1 333 333 3333 |
  | gift         | $123            |
  And we email "fbo-report" to member "c@" with subs:
  | gift         | $123                 |**
  | date         | %mdY                 |
  | donor        | Zee Zot              |
  | donorAddress | Greenfield, MA 01301 |
  | donorPhone   | +1 262 626 2626      |
  | donorEmail   | z@example.com        |
  | fullName     | Our Pub              |
  | qid          | .ZZC                 |
  And we say "status": "gift thanks|check it out" with subs:
  | coName | Our Pub |**

Scenario: A non-member donates to a sponsored organization by ACH
  Given a button code for:
  | account | secret |*
  | .ZZC    | Cc3    |
  And next captcha is "37"
  When member "?" completes "community/donate/code=TESTCODE" with:
  | amount | fullName | phone        | email | zip   | payHow | comment  | cq | ca |*
  |    123 | Zee Zot  | 262-626-2626 | z@    | 01301 |      0 | awesome! | 37 | 74 |
  Then these "people":
  | pid | fullName | phone        | email | zip   | state |*
  | 1   | Zee Zot  | +12626262626 | z@    | 01301 | MA    |
  And these "txs2":
  | xid | payee | amount | completed | deposit | pid |*
  | 1   | .ZZC  | 123    | %now      |       0 | 1   |
  And these "txs":
  | eid | xid | payer      | payee | amount | purpose    | type       |*
  | 1   | 1   | %UID_OUTER | .ZZC  | 123    | donation   | %E_OUTER   |
  | 3   | 1   | .ZZC       | cgf   | 6.15   | sponsor    | %E_AUX     |
  And count "tx_entries" is 4
  And we email "fbo-thanks" to member "z@" with subs:
  | fullName     | Zee Zot         |**
  | date         | %mdY            |
  | coName       | Our Pub         |
  | coPostalAddr | 3 C, C, FR      |
  | coPhone      | +1 333 333 3333 |
  | gift         | $123            |
  And we email "fbo-report" to member "c@" with subs:
  | gift         | $123                 |**
  | date         | %mdY                 |
  | donor        | Zee Zot              |
  | donorAddress | Greenfield, MA 01301 |
  | donorPhone   | +1 262 626 2626      |
  | donorEmail   | z@example.com        |
  | fullName     | Our Pub              |
  | qid          | .ZZC                 |
  And we say "status": "gift thanks|check it out" with subs:
  | coName | Our Pub |**

Scenario: A member donates to a sponsored organization
  Given a button code for:
  | account | secret |*
  | .ZZC    | Cc3    |
  When member ".ZZA" completes "community/donate/code=TESTCODE" with:
  | amount | comment  | period | honor  | honored |*
  |    123 | awesome! | month  | memory | Mike    |
  Then these "txs":
  | eid | xid | payer | payee | amount | purpose    | type       |*
  | 1   | 1   | .ZZA  | .ZZC  | 123    | donation   | %E_PRIME   |
  | 3   | 1   | .ZZC  | cgf   | 6.15   | sponsor    | %E_AUX     |
  And these "tx_timed":
  | id | action   | from | to   | amount | portion | purpose  | payerType    | payeeType    | period |*
  | 1  | %ACT_PAY | .ZZA | .ZZC | 123    | 0       | donation | %REF_ANYBODY | %REF_ANYBODY | month  |  
  And count "tx_entries" is 4
  And we email "fbo-thanks-member" to member "a@" with subs:
  | fullName     | Abe One         |**
  | date         | %mdY            |
  | coName       | Our Pub         |
  | coPostalAddr | 3 C, C, FR      |
  | coPhone      | +1 333 333 3333 |
  | gift         | $123 monthly    |
  And we email "fbo-report" to member "c@" with subs:
  | gift         | $123 monthly         |**
  | date         | %mdY                 |
  | donor        | Abe One              |
  | donorAddress | 1 A, A, AK           |
  | donorPhone   | +1 301 301 3001      |
  | donorEmail   | a@example.com        |
  | fullName     | Our Pub              |
  | qid          | .ZZC                 |
  And we say "status": "gift thanks" with subs:
  | coName | Our Pub |**

Scenario: A member pays a sponsored organization
  When member ".ZZA" submits "tx/pay" with:
  | op  | who     | amount | purpose | period | periods |*
  | pay | Our Pub | 123    | gift    | month  | 1       |
  Then these "txs":
  | eid | xid | payer | payee | amount | purpose | type       |*
  | 1   | 1   | .ZZA  | .ZZC  | 123    | gift    | %E_PRIME   |
  | 3   | 1   | .ZZC  | cgf   | 6.15   | sponsor | %E_AUX     |
  And these "tx_timed":
  | id | action   | from | to   | amount | portion | purpose | payerType    | payeeType    | period | periods |*
  | 1  | %ACT_PAY | .ZZA | .ZZC | 123    | 0       | gift    | %REF_ANYBODY | %REF_ANYBODY | month  | 1       |
  And count "tx_entries" is 4
  And we email "fbo-thanks-member" to member "a@" with subs:
  | fullName     | Abe One         |**
  | date         | %mdY            |
  | coName       | Our Pub         |
  | coPostalAddr | 3 C, C, FR      |
  | coPhone      | +1 333 333 3333 |
  | gift         | $123 monthly    |
  And we email "fbo-report" to member "c@" with subs:
  | gift         | $123 monthly         |**
  | date         | %mdY                 |
  | donor        | Abe One              |
  | donorAddress | 1 A, A, AK           |
  | donorPhone   | +1 301 301 3001      |
  | donorEmail   | a@example.com        |
  | fullName     | Our Pub              |
  | qid          | .ZZC                 |
  And we say "status": "gift thanks" with subs:
  | coName | Our Pub |**
