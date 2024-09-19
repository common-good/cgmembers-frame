Feature: Donate
AS a member
I WANT to donate to CGF
SO I can enjoy the rCredit system's rapid growth and be a part of that.

Setup:
  Given members:
  | uid  | fullName | address | city  | state | zip   | postalAddr | flags   |*
  | .ZZA | Abe One  | 1 A St. | Atown | AK    | 01000 | 1 A, A, AK | ok,confirmed      |
  | .ZZC | Cor Pub  | 3 C St. | Ctown | CT    | 03000 | 3 C, C, CT | ok,confirmed,co   |
  And balances:
  | uid    | balance |*
  | cgf    |       0 |
  | .ZZA   |     100 |
  | .ZZC   |     100 |

Scenario: A member donates
  Given next DO code is "whatever"
  When member ".ZZA" visits page "ccpay"
  Then we show "Donate to %PROJECT" with:
  | Donation | One Brick |
  | When      | yearly    |
  | Honoring | |
  And with choices:
  | year    | yearly |
  | quarter | quarterly |
  | month   | monthly |
  | week    | weekly |
  | day     | daily |
#  And without: (can't tell this isn't showing because it's CSS)
#  | Other amount |
  
  When member ".ZZA" completes form "ccpay" with values:
  | amtChoice | amount | period | honor  | honored |*
  |        -1 |     10 | once   | memory | Jane Do |
  Then these "txs":
  | xid | created | amount | payer | payee | purpose      |*
  |   1 | %today  |     10 | .ZZA  | cgf  | donation |
  And we say "status": "gift thanks|cggift thanks" with subs:
  | coName | %PROJECT |**
  And these "r_honors":
  | created | uid  | honor  | honored |*
  | %today  | .ZZA | memory | Jane Do |
  And we message "gift sent" to member ".ZZA" with subs:
  | amount | $10 |**
  And we message "paid you linked" to member "cgf" with subs:
  | otherName | amount | payeePurpose | aPayLink |*
  | Abe One   | $10    | donation     | ?        |
  And that link has results:
  | ~name | Abe One |
  | ~postalAddr | 1 A, A, AK |
  | Physical address: | 1 A St., Atown, AK 01000 |
#  And we tell admin "gift accepted" with subs:
#  | amount | period |*
#  |     10 |     1 |
  # and many other fields

Scenario: A member makes a recurring donation
  When member ".ZZA" completes form "ccpay" with values:
  | amtChoice | amount | period | honor  | honored |*
  |        -1 |     10 | month  | memory | Jane Do |
  Then these "tx_timed":
  | id | start  | from | to  | amount | period | purpose  |*
  |  1 | %today | .ZZA | cgf |     10 | month  | donation |
  And these "txs":
  | xid | created | amount | payer | payee    | purpose  | recursId |*
  |   1 | %today  |     10 | .ZZA  | regulars | donation |        1 |
  And we say "status": "gift thanks|cggift thanks" with subs:
  | coName | %PROJECT |**
  And these "r_honors":
  | created | uid  | honor  | honored |*
  | %today  | .ZZA | memory | Jane Do |
  And we message "gift sent" to member ".ZZA" with subs:
  | amount | $10 |**

Scenario: A member makes a new recurring donation
  Given these "tx_timed":
  | id | start     | from | to  | amount | period |*
  | 1  | %today-1d | .ZZA | cgf |     25 | year   |
  When member ".ZZA" visits page "ccpay"
  Then we show "donation replaces" with:
  | period | amt |*
  | yearly | $25 |

  When member ".ZZA" completes form "ccpay" with values:
  | amtChoice | amount | period | honor  | honored |*
  |        -1 |     10 | month  | memory | Jane Do |
  Then these "txs":
  | xid | created | amount | payer | payee    | purpose  | recursId |*
  |   1 | %today  |     10 | .ZZA  | regulars | donation | 2        |
  And we say "status": "prev gift canned"
  And we say "status": "gift thanks|cggift thanks" with subs:
  | coName | %PROJECT |**
  And these "tx_timed":
  | id | start     | from | to  | amount | period | end  |*
  | 1  | %today-1d | .ZZA | cgf |     25 | year   | %now |
  | 2  | %today    | .ZZA | cgf |     10 | month  | %NUL |

Scenario: A member makes a new recurring donation of zero
  Given these "tx_timed":
  | start     | from | to  | amount | period |*
  | %today-1d | .ZZA | cgf |     25 | year   |
  When member ".ZZA" visits page "ccpay"
  Then we show "donation replaces" with:
  | period | amt |*
  | yearly | $25 |

  When member ".ZZA" completes form "ccpay" with values:
  | amtChoice | amount | period |*
  |        -1 |      0 | year   |
  Then we say "status": "prev gift canned"
  And count "txs" is 0
  And these "tx_timed":
  | start     | from | to  | amount | period | end  |*
  | %today-1d | .ZZA | cgf |     25 | year   | %now |
  And count "tx_timed" is 1

Scenario: A company makes a recurring donation
  When member ".ZZC" completes form "ccpay" with values:
  | amtChoice | amount | period | honor  | honored |*
  |        -1 |     10 | month  | memory | Jane Do |
  Then these "txs":
  | xid | created | amount | payer | payee    | purpose  |*
  |   1 | %today  |     10 | .ZZC  | regulars | donation |
  And we say "status": "gift thanks|cggift thanks" with subs:
  | coName | %PROJECT |**
  
Scenario: A member donates with insufficient funds
  When member ".ZZA" completes form "ccpay" with values:
  | amtChoice | amount | period | honor  | honored |*
  |        -1 |    200 | once   | memory | Jane Do |
  Then we say "status": "gift thanks|cggift thanks" with subs:
  | coName | %PROJECT |**
  And we say "status": "gift transfer later"
  And these "tx_requests":
  | nvid | created | amount | payer | payee | purpose  | flags        | status   |*
  |    1 | %today  |    200 | .ZZA  | cgf   | donation | gift,funding | approved |
  And these "r_honors":
  | created | uid  | honor  | honored |*
  | %today  | .ZZA | memory | Jane Do |
  And we tell "ctty" CO "gift" with subs:
  | amount | period |*
  |    200 | once   |
