Feature: Tx
AS a member
I WANT to transfer credit to or from another member (acting on their own behalf)
SO I can buy and sell stuff.
 We will eventually need variants or separate feature files for neighbor (member of different community within the region) to member, etc.
 And foreigner (member on a different server) to member, etc.

Setup:
  Given members:
  | uid  | fullName | address | city  | state  | zip | country  | postalAddr | floor | flags      |*
  | .ZZA | Abe One  | 1 A St. | Atown | Alaska | 01000 | US     | 1 A, A, AK |  -250 | ok,confirmed,debt         |
  | .ZZB | Bea Two  | 2 B St. | Btown | Utah   | 02000 | US     | 2 B, B, UT |     0 | ok,confirmed         |
  | .ZZC | Our Pub  | 3 C St. | Ctown | Cher   |       | France | 3 C, C, FR |     0 | ok,confirmed,co      |
  And relations:
  | main | agent | permission |*
  | .ZZA | .ZZB  | buy        |
  | .ZZB | .ZZA  | read       |
  | .ZZC | .ZZB  | buy        |
  | .ZZC | .ZZA  | sell       |
  Then balances:
  | uid  | balance |*
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |       0 |

# (rightly fails, so do this in a separate feature) Variants: with/without an agent
#  | ".ZZA" | # member to member (pro se) |
#  | ".ZZA" | # agent to member           |

Scenario: A member asks to charge another member for goods
  When member ".ZZA" completes form "tx/charge" with values:
  | op     | who     | amount | goods | purpose |*
  | charge | Bea Two | 100    | %FOR_GOODS     | labor   |
  Then we scrip "tx" with subs:
  | field | question            | selfErr | payDesc | chargeDesc |*
  | who   | %_%amount to %name? | self-tx | Pay     | Charge     |
  # choice between Pay and Charge gets set in JS

Scenario: A member confirms request to charge another member
  When member ".ZZA" confirms form "tx/charge" with values:
  | op     | who     | amount | purpose |*
  | charge | Bea Two | 100    | labor   |
  Then we say "status": "report tx|balance unchanged" with subs:
  | did     | otherName | amount |*
  | charged | Bea Two   | $100   |
  And we message "invoiced you" to member ".ZZB" with subs:
  | otherName | amount | purpose |*
  | Abe One   | $100   | labor   |
  And invoices:
  | nvid | created | status      | amount | payer | payee | for   |*
  |    1 | %today  | %TX_PENDING |    100 | .ZZB  | .ZZA | labor |
  And balances:
  | uid  | balance |*
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |       0 |

Scenario: A member asks to pay another member for goods
  When member ".ZZA" completes form "tx/pay" with values:
  | op  | who     | amount | goods | purpose |*
  | pay | Bea Two | 100    | %FOR_GOODS     | labor   |
  Then we scrip "tx" with subs:
  | field | question            | selfErr | payDesc | chargeDesc |*
  | who   | %_%amount to %name? | self-tx | Pay     | Charge     |

Scenario: A member asks to pay another member for loan/reimbursement
  When member ".ZZA" completes form "tx/pay" with values:
  | op  | who     | amount | goods | purpose |*
  | pay | Bea Two | 100    | %FOR_NONGOODS | loan    |
  Then we scrip "tx" with subs:
  | field | question            | selfErr | payDesc | chargeDesc |*
  | who   | %_%amount to %name? | self-tx | Pay     | Charge     |
  
Scenario: A member confirms request to pay another member
  When member ".ZZA" confirms form "tx/pay" with values:
  | op  | who     | amount | goods | purpose |*
  | pay | Bea Two | 100    | %FOR_GOODS     | labor   |
  Then we say "status": "report tx" with subs:
  | did    | otherName | amount |*
  | paid   | Bea Two   | $100   |
  And we notice "paid you" to member ".ZZB" with subs:
  | created | fullName | otherName | amount | payeePurpose |*
  | %today  | Bea Two  | Abe One    | $100   | labor        |
  And transactions:
  | xid | created | amount | payer | payee | purpose      | taking |*
  |   1 | %today  |    100 | .ZZA  | .ZZB | labor        | 0      |
  And balances:
  | uid  | balance |*
  | .ZZA |    -100 |
  | .ZZB |     100 |
  | .ZZC |       0 |
  
Scenario: A member confirms request to pay another member a lot
  Given balances:
  | uid  | balance       |*
  | .ZZB | %MAX_AMOUNT |
  When member ".ZZB" confirms form "tx/pay" with values:
  | op  | who     | amount        | goods | purpose |*
  | pay | Our Pub | %MAX_AMOUNT | %FOR_GOODS     | food    |
  Then transactions:
  | xid | created | amount        | payer | payee | purpose      | taking |*
  |   1 | %today  | %MAX_AMOUNT | .ZZB  | .ZZC | food         | 0      |
  
Scenario: A member confirms request to pay a member company
  Given next DO code is "whatever"
  When member ".ZZA" confirms form "tx/pay" with values:
  | op  | who     | amount | goods | purpose |*
  | pay | Our Pub | 100    | %FOR_GOODS     | stuff   |
  Then we say "status": "report tx" with subs:
  | did    | otherName | amount |*
  | paid   | Our Pub   | $100   |
  And we notice "paid you linked" to member ".ZZC" with subs:
  | created | fullName | otherName | amount | payeePurpose | aPayLink |*
  | %today  | Our Pub  | Abe One   | $100 | stuff | ? |
  And that "notice" has link results:
  | ~name | Abe One |
  | ~postalAddr | 1 A, A, AK |
  | Physical address: | 1 A St., Atown, AK 01000 |
  And transactions:
  | xid | created | amount | payer | payee | purpose      | taking |*
  |   1 | %today  |    100 | .ZZA  | .ZZC | stuff        | 0      |
  And balances:
  | uid  | balance |*
  | .ZZA |    -100 |
  | .ZZB |       0 |
  | .ZZC |     100 |

#NO. Duplicates are never flagged on web interface.
#Scenario: A member confirms request to pay the same member the same amount
#  Given member ".ZZA" confirms form "tx/pay" with values:
#  | op  | who     | amount | goods | purpose |*
#  | pay | Bea Two | 100    | %FOR_GOODS     | labor   |  
#  When member ".ZZA" confirms form "tx/pay" with values:
#  | op  | who     | amount | goods | purpose |*
#  | pay | Bea Two | 100    | %FOR_GOODS     | labor   |
#  Then we say "error": "duplicate transaction" with subs:
#  | op   |*
#  | paid |
  
#Scenario: A member confirms request to charge the same member the same amount
#  Given member ".ZZA" confirms form "tx/charge" with values:
#  | op     | who     | amount | goods | purpose |*
#  | charge | Bea Two | 100    | %FOR_GOODS     | labor   |  
#  When member ".ZZA" confirms form "tx/charge" with values:
#  | op     | who     | amount | goods | purpose |*
#  | charge | Bea Two | 100    | %FOR_GOODS     | labor   |
#  Then we say "error": "duplicate transaction" with subs:
#  | op      |*
#  | charged |

#Scenario: A member leaves goods blank
#  Given member ".ZZA" confirms form "tx/pay" with values:
#  | op  | who     | amount | goods | purpose |*
#  | pay | Bea Two | 100    |       | labor   |  
#  Then we say "error": "required field" with subs:
#  | field |*
#  | "For" |

Skip this is now allowed, as an implicit invitation (3 scenarios)
Scenario: A member asks to charge another member before making an rCard purchase
  Given member ".ZZA" has no photo ID recorded
  When member ".ZZA" completes form "tx/charge" with values:
  | op     | who     | amount | goods | purpose |*
  | charge | Bea Two | 100    | %FOR_GOODS     | labor   |
  Then we say "error": "no photoid"

Scenario: A member asks to charge another member before the other has made an rCard purchase
  Given member ".ZZB" has no photo ID recorded
  When member ".ZZA" completes form "tx/charge" with values:
  | op     | who     | amount | goods | purpose |*
  | charge | Bea Two | 100    | %FOR_GOODS     | labor   |
  Then we say "error": "other no photoid" with subs:
  | who     |*
  | Bea Two |
  
Scenario: A member confirms payment of an invoice before making a Common Good Card purchase
  Given member ".ZZA" confirms form "tx/pay" with values:
  | op  | who     | amount | goods      | purpose |*
  | pay | Bea Two | 100    | %FOR_GOODS | labor   |  
  Then we say "error": "first at home"
  
Skip (not sure about this feature)
Scenario: A member asks to pay another member before the other has made a Common Good Card purchase
  Given member ".ZZB" has no photo ID recorded
  When member ".ZZA" completes form "tx/pay" with values:
  | op  | who     | amount | goods | purpose |*
  | pay | Bea Two | 100    | %FOR_GOODS     | labor   |
  Then we say "error": "other no photoid" with subs:
  | who     |*
  | Bea Two |
Resume

Scenario: A new member asks to pay another member before making a Common Good Card purchase
  Given member ".ZZA" is unconfirmed
  When member ".ZZA" completes form "tx/pay" with values:
  | op  | who     | amount | goods | purpose |*
  | pay | Bea Two | 100    | %FOR_GOODS     | labor   |
  Then we say "status": "first at home|when resolved"

Scenario: A member pays another member repeatedly
  Given these "tx_timed":
  | id | from | to   | amount | period | purpose | start | end | action | duration |*
  |  3 | 0    | 0    |      0 | week   | bump-id | %now  |     | pay    | once     |
  When member ".ZZA" confirms form "tx/pay" with values:
  | op  | who     | amount | purpose | period | periods |*
  | pay | Bea Two | 100    |  labor  | week   |       1 |
  Then we say "status": "report tx|repeats" with subs:
  | did  | otherName | amount | often  |*
  | paid | Bea Two   | $100   | weekly |
  And we notice "paid you" to member ".ZZB" with subs:
  | created | fullName | otherName | amount | payeePurpose |*
  | %today  | Bea Two  | Abe One    | $100   | labor        |
  And transactions:
  | xid | created | amount | payer | payee | purpose      | taking | recursId |*
  |   1 | %today  |    100 | .ZZA  | .ZZB  | labor        | 0      |        4 |
  And date field "created" rounded "no" in "tx_hdrs" record "1" (id field "xid")
  And these "tx_timed":
  | id | from | to   | amount | period | purpose | start     | end | action | duration |*
  |  4 | .ZZA | .ZZB |    100 | week   | labor   | %daystart |     | pay    | once     |
  And date field "start" rounded "yes" in "tx_timed" record "4" (id field "id")
  And field "tx_hdrs/xid/1/created" is ">=" field "tx_timed/id/1/start"

Scenario: A member pays another member later
  When member ".ZZA" confirms form "tx/pay" with values:
  | op  | who     | amount | purpose | start   |*
  | pay | Bea Two | 100    |  labor  | %mdy+3d |
  Then we say "status": "thing scheduled" with subs:
  | thing   |*
  | payment |
  And count "txs" is 0
  And these "tx_timed":
  | id | from | to   | amount | period | purpose | start        | end | action | duration |*
  |  1 | .ZZA | .ZZB |    100 | once   | labor   | %daystart+3d |     | pay    | once     |
