Feature: Offline
AS a company agent
I WANT to accept transactions offline
SO my company can sell stuff, give refunds, and trade USD for rCredits even when no internet is available

and I WANT those transactions to be reconciled when an internet connection becomes available again
SO my company's online account records are not incorrect for long.

Setup:
  Given members:
  | uid  | fullName   | email | cc  | cc2  | floor | flags                | activated |*
  | .ZZA | Abe One    | a@    | ccA | ccA2 |  -250 | ok,confirmed,debt    | %today-2y |
  | .ZZB | Bea Two    | b@    | ccB | ccB2 |  -250 | ok,confirmed,debt    | %today-2y |
  | .ZZC | Corner Pub | c@    | ccC |      |  -250 | ok,confirmed,co,debt | %today-2y |
  | .ZZD | Dee Four   | d@    | ccD | ccD2 |     0 | ok,confirmed         | %today-2y |
  | .ZZE | Eve Five   | e@    | ccE | ccE2 |     0 | ok,confirmed,secret  | %today-2y |
  | .ZZF | Far Co     | f@    | ccF |      |     0 | ok,confirmed,co      | %today-2y |
  And devices:
  | uid  | code |*
  | .ZZC | devC |
  And selling:
  | uid  | selling         |*
  | .ZZC | this,that,other |
  And company flags:
  | uid  | coFlags      |*
  | .ZZC | refund,r4usd |
  And relations:
  | main | agent | num | permission |*
  | .ZZC | .ZZA  |   1 | buy        |
  | .ZZC | .ZZB  |   2 | scan       |
  | .ZZC | .ZZD  |   3 | read       |
  | .ZZF | .ZZE  |   1 | sell       |
  And transactions: 
  | xid | created   | amount | payer | payee | purpose |*
  | 4   | %today-6m |    250 | ctty | .ZZF | stuff   |
  Then balances:
  | uid  | balance |*
  | ctty |    -250 |
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |       0 |
  | .ZZF |     250 |

Scenario: A cashier charged someone offline
  When reconciling "C:A" on "devC" charging ".ZZB,ccB" $100 for "goods": "food" at "%now-1h" force 1
  Then we respond ok txid 5 created "%now-1h" balance -100 saying:
  | did     | otherName | amount | why   |*
  | charged | Bea Two   | $100   | goods |
# NOPE  And with proof of agent "C:A" amount 100.00 created "%now-1h" member ".ZZB" code "ccB"
  And we notice "new charge" to member ".ZZB" with subs:
  | created | fullName | otherName  | amount | payerPurpose |*
  | %today  | Bea Two  | Corner Pub | $100   | food         |
  And balances:
  | uid  | balance |*
  | ctty |    -250 |
  | .ZZA |       0 |
  | .ZZB |    -100 |
  | .ZZC |     100 |

Scenario: A cashier charged someone offline and they have insufficient balance
  Given transactions: 
  | xid | created | amount | payer | payee | purpose |*
  | 5   | %today  |    200 | .ZZB | .ZZC | cash    |
  Then balances:
  | uid  | balance |*
  | .ZZB |    -200 |
  | .ZZC |     200 |
  When reconciling "C:A" on "devC" charging ".ZZB,ccB" $100 for "goods": "food" at "%now-1h" force 1
  Then we respond ok txid 6 created "%now-1h" balance -300
  And we notice "new charge" to member ".ZZB" with subs:
  | created | fullName | otherName  | amount | payerPurpose |*
  | %today  | Bea Two  | Corner Pub | $100   | food         |
  And balances:
  | uid  | balance |*
  | ctty |    -250 |
  | .ZZA |       0 |
  | .ZZB |    -300 |
  | .ZZC |     300 |

Scenario: A cashier charged someone offline but it actually went through
  Given agent "C:A" asks device "devC" to charge ".ZZB,ccB" $100 for "goods": "food" at "%now-1h"
  When reconciling "C:A" on "devC" charging ".ZZB,ccB" $100 for "goods": "food" at "%now-1h" force 1
  Then we respond ok txid 5 created "%now-1h" balance -100
  #And we notice nothing
  And balances:
  | uid  | balance |*
  | ctty |    -250 |
  | .ZZA |       0 |
  | .ZZB |    -100 |
  | .ZZC |     100 |

Scenario: A cashier declined to charge someone offline and it didn't go through
  When reconciling "C:A" on "devC" charging ".ZZB,ccB" $100 for "goods": "food" at "%now-1h" force -1
  Then we respond ok txid 0 created "" balance 0
  #And we notice nothing
  And balances:
  | uid  | balance |*
  | ctty |    -250 |
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |       0 |

Scenario: A cashier canceled offline a supposedly offline charge that actually went through
  Given agent "C:A" asks device "devC" to charge ".ZZB,ccB" $100 for "goods": "food" at "%now-1h"
  When reconciling "C:A" on "devC" charging ".ZZB,ccB" $100 for "goods": "food" at "%now-1h" force -1
  Then we respond ok txid 6 created %now balance 0
  And with undo "5"
  And we notice "new charge" to member ".ZZB" with subs:
  | created | fullName | otherName  | amount | payerPurpose |*
  | %today  | Bea Two  | Corner Pub | $100   | food         |
  And we notice "new refund" to member ".ZZB" with subs:
  | created | fullName | otherName  | amount | payerPurpose |*
  | %today  | Bea Two  | Corner Pub | $100   | food         |
  And balances:
  | uid  | balance |*
  | ctty |    -250 |
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |       0 |

Scenario: A cashier canceled offline a supposedly offline charge that actually went through, but customer is broke
  Given transactions: 
  | xid | created | amount | payer | payee | purpose |*
  | 5   | %today  |    500 | ctty | .ZZC | growth  |
  Then count "txs" is 2

  When agent "C:A" asks device "devC" to charge ".ZZB,ccB" $-100 for "goods": "refund" at "%now-1n"
  Then transactions: 
  | xid | created | amount | payer | payee | purpose | taking |*
  | 6   | %now-1n |   -100 | .ZZB | .ZZC | refund  |      1 |
  And count "txs" is 3

  Given transactions: 
  | xid | created | amount | payer | payee | purpose |*
  | 7   | %today  |    300 | .ZZB | .ZZA | cash    |
  When reconciling "C:A" on "devC" charging ".ZZB,ccB" $-100 for "goods": "refund" at "%now-1n" force -1
  Then we respond ok txid 8 created %now balance -300
  And with undo "6"
  And we notice "new refund" to member ".ZZB" with subs:
  | created | fullName | otherName  | amount | payerPurpose |*
  | %now    | Bea Two  | Corner Pub | $100   | refund       |
  And we notice "new charge" to member ".ZZB" with subs:
  | created | fullName | otherName  | amount | payerPurpose |*
  | %now    | Bea Two  | Corner Pub | $100   | refund       |
  And balances:
  | uid  | balance |*
  | ctty |    -750 |
  | .ZZA |     300 |
  | .ZZB |    -300 |
  | .ZZC |     500 |

Scenario: Device sends correct old proof for legit tx after member loses card, with app offline
  Given members have:
  | uid  | cardCode |*
  | .ZZB | ccB2     |
  // member just changed cardCode
  When reconciling "C:A" on "devC" charging ".ZZB,ccB" $100 for "goods": "food" at "%now-1h" force 1
  Then we respond ok txid 5 created "%now-1h" balance -100 saying:
  | did     | otherName | amount | why   |*
  | charged | Bea Two   | $100   | goods |

Scenario: Device sends correct old proof for legit tx after member loses card, with app online
  Given members have:
  | uid  | cardCode |*
  | .ZZB | ccB2     |
  // member reported lost card, we just changed cardCode, now the member (or someone) tries to use the card with app online:
  When reconciling "C:A" on "devC" charging ".ZZB,ccB" $100 for "goods": "food" at "%now-1h" force 0
  Then we return error "bad proof"


Scenario: Device sends correct old proof for legit tx after member loses card, with tx date after the change
  Given members have:
  | uid  | cardCode |*
  | .ZZB | ccB2     |
  // member reported lost card, we just changed cardCode, now the member (or someone) tries to use the card with app online:
	// (use +1d not +1h, because t\hitServer rounds down to nearest day)
  When reconciling "C:A" on "devC" charging ".ZZB,ccB" $100 for "goods": "food" at "%now+1d" force 1
  Then we return error "bad proof"
