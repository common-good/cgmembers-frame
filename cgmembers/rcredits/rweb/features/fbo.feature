Feature: FBO
AS a sponsored member company or administrator
I WANT to track payments to and from non-members
SO I can accept donations and make payments for a fiscally-sponsored organization or a member nonprofit.

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
  | purpose   | %FS_NOTE |
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
  | Full Name   | |
  | Postal Addr | |
  | City        | |
  | State       | |
  | Zip         | |
  | Amount      | |
  | For         | |
  | Category    | |
  And without:
  | Member      | Non-member |
  
  Given members have:
  | uid  | flags    |*
  | .ZZA | ok,admin |
  And member ".ZZA" has admin permissions: "seeAccts chargeFrom nonmemberTx"
  When member "C:A" visits "tx/charge"
  Then we show "Charge" with:
  | Member      | Non-member |
  | Full Name   | |
  | Postal Addr | |
  When member "C:A" submits "tx/charge" with:
  | op     | fbo | fullName | email | address | city | state | zip   | amount | purpose | comment | cat         |*
  | charge | 1   | Dee Forn | d@    | 4 Fr St | Fton | MA    | 01004 | 100    | grant   |         | D-FBO       |
  Then we scrip "tx" with subs:
  | field | question            | selfErr | payDesc | chargeDesc | fbo | admin |*
  | who   | %_%amount to %name? | self-tx | Pay     | Charge     | 1   | 1     |
  # choice between Pay and Charge gets set in JS
  And we say "status": "info saved"
  And these "txs":
  | eid | xid | payer      | payee | amount | purpose  | cat1        | cat2        | type     |*
  |   1 | 1   | %UID_OUTER | .ZZC  | 100    | grant    |             | D-FBO       | %E_OUTER |
  |   3 | 1   | .ZZC       | cgf   | 5      | %FS_NOTE | D-FBO       | FS-FEE      | %E_AUX   |
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
  And we email "fbo-thanks-nonmember" to member "d@" with subs:
  | fullName     | Dee Forn        |**
  | date         | %mdY            |
  | coName       | Our Pub         |
  | coPostalAddr | 3 C, C, FR      |
  | coPhone      | +1 333 333 3333 |
  | amount       | $100            |
  | noFrame      | 1               |
  And we email "gift-report" to member ".ZZC" with subs:
  | amount      | $100                 |**
  | date        | %mdY                 |
  | fromName    | Dee Forn             |
  | fromAddress | 4 Fr St, Fton, MA 01004 |
  | fromPhone   |                      |
  | fromEmail   | d@example.com        |
  | note        |                      |

Scenario: A non-member donates to Common Good
  Given members have:
  | uid  | flags    |*
  | .ZZA | ok,admin |
  And member ".ZZA" has admin permissions: "seeAccts manageAccts chargeFrom nonmemberTx"
  When member "cgf:A" visits "tx/charge"
  Then we show "Charge" with:
  | Full Name   | |
  | Postal Addr | |
  | City        | |
  | State       | |
  | Zip         | |
  | Phone       | |
  | Email       | |
  | Amount      | donation |
  | For         | |
  | Method      | |
  | Category    | |
  And with:
  | Member      | Non-member |

  When member "cgf:A" submits "tx/charge" with:
  | op     | fbo | fullName | email | address | city | state | zip   | amount | isGift | purpose | comment | cat         |*
  | charge | 1   | Dee Forn | d@    | 4 Fr St | Fton | MA    | 01004 | 100    | 1      | grant   |         | D-FBO       |
  Then we scrip "tx" with subs:
  | field | question            | selfErr | payDesc | chargeDesc | fbo | admin | hasCats |*
  | who   | %_%amount to %name? | self-tx | Pay     | Charge     | 0   | 1     | 1       |
  # choice between Pay and Charge gets set in JS
  And we say "status": "info saved"
  And these "txs":
  | eid | xid | payer      | payee | amount | purpose  | cat1        | cat2        | type     |*
  |   1 | 1   | %UID_OUTER | cgf   | 100    | grant    |             | D-FBO       | %E_OUTER |
  And these "txs2":
  | xid | payee | amount | completed | deposit | pid |*
  | 1   | cgf   | 100    | %now      | %now    | 1   |
  And these "people":
  | pid | fullName | address | city | state | zip   |*
  | 1   | Dee Forn | 4 Fr St | Fton | MA    | 01004 |
  And balances:
  | uid  | balance |*
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |       0 |
  | cgf  |     100 |
  And we email "cggift-thanks-nonmember" to member "d@" with subs:
  | fullName     | Dee Forn        |**
  | date         | %mdY            |
  | coName       | %PROJECT        |
  | coPostalAddr | %CGF_POSTALADDR |
  | coPhone      | %CGF_PHONE      |
  | amount       | $100            |
  | noFrame      | 1               |
  And we email "gift-report" to member "cgf" with subs:
  | amount      | $100                 |**
  | date        | %mdY                 |
  | fromName    | Dee Forn             |
  | fromAddress | 4 Fr St, Fton, MA 01004 |
  | fromPhone   |                      |
  | fromEmail   | d@example.com        |
  | note        |                      |

Scenario: A sponsored member pays a nonmember
  When member "C:A" visits "tx/pay"
  Then we show "Pay" with:
  | Full Name   | |
  | Postal Addr | |
  | City        | |
  | State       | |
  | Zip         | |
  | Amount      | |
  | For         | |
  | Category    | |
  And without:
  | Member      | Non-member |

  Given members have:
  | uid  | flags    |*
  | .ZZA | ok,admin |
  And member ".ZZA" has admin permissions: "seeAccts payFrom nonmemberTx"
  When member "C:A" visits "tx/pay"
  Then we show "Pay" with:
  | Member      | Non-member |
  | Full Name   | |
  | Postal Addr | |
  
  When member "C:A" submits "tx/pay" with:
  | op  | fbo | fullName | address | city | state | zip   | amount | purpose | cat       |*
  | pay | 1   | Dee Forn | 4 Fr St | Fton | MA    | 01004 | 100    | labor   | FBO-LABOR |
  Then we scrip "tx" with subs:
  | field | question            | selfErr | payDesc | chargeDesc |*
  | who   | %_%amount to %name? | self-tx | Pay     | Charge     |
  # choice between Pay and Charge gets set in JS
  And we say "status": "info saved"
  And these "txs":
  | xid | payer      | payee | amount | purpose | cat2      | type     |*
  | 1   | %UID_OUTER | .ZZC  | -100   | labor   | FBO-LABOR | %E_OUTER |
  And these "txs2":
  | xid | payee | amount | completed | deposit | pid |*
  | 1   | .ZZC  | -100   | %now      |    %now | 1   |
  #if sometime we allow fbo members to request payments, change the deposit date above to 0 (already that way in the code)
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
  | op     | fbo | who  | amount | purpose | cat         |*
  | charge |   1 | .ZZB |    100 | grant   | D-FBO       |
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
  | nvid | created | status      | amount | payer | payee | for   | cat         |*
  |    1 | %today  | %TX_PENDING |    100 | .ZZB  | .ZZC  | grant | D-FBO       |
  And balances:
  | uid  | balance |*
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |       0 |

  When member ".ZZB" confirms form "handle-invoice/nvid=1&code=TESTDOCODE" with values:
  | op   | ret | nvid | payAmount | payer | payee | purpose | created |*
  | pay  |     |    1 |       100 | .ZZB  | .ZZC  | grant   | %today  |
  Then these "txs":
  | xid | created | amount | payer | payee | purpose | taking | relType | rel | cat2        |*
  |   1 | %today  |    100 | .ZZB  | .ZZC  | grant   | 0      | I       | 1   | D-FBO       |

Scenario: A member pays a sponsored member everything they have
  When member ".ZZB" submits "tx/pay" with:
  | op  | fbo | who  | amount | purpose|*
  | pay |   0 | .ZZC |    250 | my all |
  Then these "txs":
  | eid | xid | created | amount | payer | payee | purpose  | taking | cat2        |*
  |   1 |   1 | %today  |    250 | .ZZB  | .ZZC  | my all   | 0      | D-FBO       |
  |   3 |   1 | %today  |  12.50 | .ZZC  | cgf   | %FS_NOTE | 0      | FS-FEE      |

Scenario: A sponsored member views their transaction history
  Given these "txs":
  | xid | payer      | payee | amount | purpose | cat2        | type     | agt2 |*
  | 1   | %UID_OUTER | .ZZC  | 100    | grant   | D-FBO       | %E_OUTER | .ZZB |
  | 2   | %UID_OUTER | .ZZC  | -200   | labor   | FBO-LABOR   | %E_OUTER | .ZZB |
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
  | Tx | Date | Name                  | Purpose | Amount  | Balance |
  | 2  | %mdy | Eva Fivn (non-member) | labor   | -200.00 | -100.00 |
  | 1  | %mdy | Dee Forn (non-member) | grant   |  100.00 |  100.00 |

  When member "C:A" visits "history/transaction/xid=1"
  Then we show "Transaction #1 Detail" with:
  | Date        | %mdY |
  | Amount      | 100 |
  | For         | grant |
  | From        | Dee Forn (non-member) (by Bea Two) * |
  | Postal Addr | 4 D St, Dton, MA 01004 |
  | Category    | I: Donations |
  | Channel     | Web |

Scenario: A non-member donates to a sponsored organization by credit card
  Given button code "buttonCode" for:
  | account | secret |*
  | .ZZC    | Cc3    |
  When member "?" visits "community/donate/code=%buttonCode"
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
  And var "code" encrypts:
  | type | item     | pid | period | amount | coId   |*
  | fbo  | donation | 1   | once   | 123.00 | NEWZZC |
  When member "?" completes "community/donate/code=%buttonCode" with:
  | amount | fullName | phone        | email | zip   | payHow | comment  | cq | ca |*
  |    123 | Zee Zot  | 262-626-2626 | z@    | 01301 |      1 | awesome! | 37 | 74 |
  Then these "people":
  | pid | fullName | phone        | email | zip   | state |*
  | 1   | Zee Zot  | +12626262626 | z@    | 01301 | MA    |

  And we redirect to "https://www.paypal.com/donate"
  And return URL "/community/donate/op=done&code=%code"
  
  When member "?" visits "community/donate/op=done&code=%code"
  Then these "txs2":
  | xid | payee | amount | completed | deposit | pid |*
  | 1   | .ZZC  | 123    | %now      |    %now | 1   |
  And these "txs":
  | eid | xid | payer      | payee | amount | purpose  | type       |*
  | 1   | 1   | %UID_OUTER | .ZZC  | 123    | donation | %E_OUTER   |
  | 3   | 1   | .ZZC       | cgf   | 6.15   | %FS_NOTE | %E_AUX     |
  | 4   | 1   | .ZZC       | cgf   | 3.69   | cc fee   | %E_XFEE    |
  And we email "fbo-thanks-nonmember" to member "z@" with subs:
  | fullName     | Zee Zot         |**
  | date         | %mdY            |
  | coName       | Our Pub         |
  | coPostalAddr | 3 C, C, FR      |
  | coPhone      | +1 333 333 3333 |
  | amount       | $123            |
  | noFrame      | 1               |
  And we email "gift-report" to member ".ZZC" with subs:
  | amount       | $123                 |**
  | date         | %mdY                 |
  | fromName     | Zee Zot              |
  | fromAddress  | Greenfield, MA 01301 |
  | fromPhone    | +1 262 626 2626      |
  | fromEmail    | z@example.com        |
  | note         | awesome!             |
  And we say "status": "gift thanks|check it out" with subs:
  | coName | Our Pub |**

Scenario: A non-member donates to a sponsored organization by ACH
  Given button code "buttonCode" for:
  | account | secret |*
  | .ZZC    | Cc3    |
  And next captcha is "37"
  When member "?" completes "community/donate/code=%buttonCode" with:
  | amount | fullName | phone        | email | zip   | payHow | comment  | cq | ca |*
  |    123 | Zee Zot  | 262-626-2626 | z@    | 01301 |      0 | awesome! | 37 | 74 |
  Then these "people":
  | pid | fullName | phone        | email | zip   | state |*
  | 1   | Zee Zot  | +12626262626 | z@    | 01301 | MA    |
  And these "txs2":
  | xid | payee | amount | completed | deposit | pid |*
  | 1   | .ZZC  | 123    | %now      |       0 | 1   |
  And these "txs":
  | eid | xid | payer      | payee | amount | purpose            | type       |*
  | 1   | 1   | %UID_OUTER | .ZZC  | 123    | donation           | %E_OUTER   |
  | 3   | 1   | .ZZC       | cgf   | 6.15   | %FS_NOTE | %E_AUX     |
  And count "tx_entries" is 4
  And we email "fbo-thanks-nonmember" to member "z@" with subs:
  | fullName     | Zee Zot         |**
  | date         | %mdY            |
  | coName       | Our Pub         |
  | coPostalAddr | 3 C, C, FR      |
  | coPhone      | +1 333 333 3333 |
  | amount       | $123            |
  | noFrame      | 1               |
  And we email "gift-report" to member ".ZZC" with subs:
  | amount       | $123                 |**
  | date         | %mdY                 |
  | fromName     | Zee Zot              |
  | fromAddress  | Greenfield, MA 01301 |
  | fromPhone    | +1 262 626 2626      |
  | fromEmail    | z@example.com        |
  | note         | awesome!             |
  And we say "status": "gift thanks|check it out" with subs:
  | coName | Our Pub |**

Scenario: A member donates to a sponsored organization
  Given button code "buttonCode" for:
  | account | secret |*
  | .ZZC    | Cc3    |
  When member ".ZZA" completes "community/donate/code=%buttonCode" with:
  | amount | comment  | period | honor  | honored |*
  |    123 | awesome! | month  | memory | Mike    |
  Then these "txs":
  | eid | xid | payer | payee | amount | purpose            | type       |*
  | 1   | 1   | .ZZA  | .ZZC  | 123    | donation           | %E_PRIME   |
  | 3   | 1   | .ZZC  | cgf   | 6.15   | %FS_NOTE | %E_AUX     |
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
  | amount       | $123 monthly    |
  And we email "gift-report" to member ".ZZC" with subs:
  | amount       | $123 monthly         |**
  | date         | %mdY                 |
  | fromName     | Abe One              |
  | fromAddress  | 1 A, A, AK           |
  | fromPhone    | +1 301 301 3001      |
  | fromEmail    | a@example.com        |
  | note         |                      |
  And we say "status": "gift thanks" with subs:
  | coName | Our Pub |**

Scenario: A member pays a sponsored organization
  When member ".ZZA" submits "tx/pay" with:
  | op  | who     | amount | purpose | period | periods | isGift |*
  | pay | Our Pub | 123    | gift    | month  | 1       | 1      |
  Then these "txs":
  | eid | xid | payer | payee | amount | purpose  | type       | cat1        | cat2        |*
  | 1   | 1   | .ZZA  | .ZZC  | 123    | gift     | %E_PRIME   |             | D-FBO       |
  | 3   | 1   | .ZZC  | cgf   | 6.15   | %FS_NOTE | %E_AUX     | D-FBO       | FS-FEE      |
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
  | amount       | $123 monthly    |
  And we email "gift-report" to member ".ZZC" with subs:
  | amount       | $123 monthly         |**
  | date         | %mdY                 |
  | fromName     | Abe One              |
  | fromAddress  | 1 A, A, AK           |
  | fromPhone    | +1 301 301 3001      |
  | fromEmail    | a@example.com        |
  | note         |                      |
  And we say "status": "gift thanks" with subs:
  | coName | Our Pub |**

Scenario: a sponsored organization moves credit to the bank
  Given members have:
  | uid  | balance |*
  | .ZZC | 100     |
  When member ".ZZC" visits "get"
  Then we show "Transfer Funds" with:
  | Pending  | You have no pending bank transfer requests. |
  | Balance  | $100 with a Credit Line of $0 |
  | Amount $ |  |
  | Category |  |
  | CG   Bank | |
  And without:
  | Bank   CG | |                                        
  
  When member ".ZZC" completes form "get" with values:
  | op  | amount | cat       |*
  | put |     86 | FBO-LABOR |
  Then these "txs":
  | xid | payer     | payee | amount | cat1 | cat2      |*
  | 1   | %UID_BANK | .ZZC  |    -86 | %NUL | FBO-LABOR |
  Then these "txs2":
  | payee | amount | created   | completed | channel | xid |*
  |  .ZZC |    -86 | %today    | %today    | %TX_WEB |   1 |
  And we say "status": "banked" with subs:
  | action  | tofrom  | amount | why             |*
  | deposit | to      | $86    | as soon as possible |
  And balances:
  | uid  | balance |*
  | .ZZC |      14 |
  And we message "banked" to member ".ZZC" with subs:
  | action  | tofrom | amount | why             |*
  | deposit | to     | $86    | as soon as possible |