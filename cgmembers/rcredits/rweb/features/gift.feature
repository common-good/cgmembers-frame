Feature: Gift
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
  When member ".ZZA" visits page "community/donate"
  Then we show "Donate to %PROJECT" with:
  | Donation | One Brick |
  | When      | How often? |
  | Honoring | |
  And with choices:
  | 0 | How often? |
  | Y | Yearly |
  | Q | Quarterly |
  | M | Monthly |
  | W | Weekly |
#  And without: (can't tell this isn't showing because it's CSS)
#  | Other amount |
  
  When member ".ZZA" completes form "community/donate" with values:
  | amtChoice | amount | period | honor  | honored |*
  |        -1 |     10 |      X | memory | Jane Do |
  Then transactions:
  | xid | created | amount | from | to   | purpose      |*
  |   1 | %today  |     10 | .ZZA | cgf  | donation |
  And we say "status": "gift successful"
  And these "honors":
  | created | uid  | honor  | honored |*
  | %today  | .ZZA | memory | Jane Do |
  And we notice "gift sent" to member ".ZZA" with subs:
  | amount | rewardAmount |*
  |    $10 |        $0.50 | 
  And we notice "new payment linked" to member "cgf" with subs:
  | otherName | amount | payeePurpose | aPayLink |*
  | Abe One   | $10    | donation     | ?        |
  And that "notice" has link results:
  | ~name | Abe One |
  | ~postalAddr | 1 A, A, AK |
  | Physical address: | 1 A St., Atown, AK 01000 |
#  And we tell admin "gift accepted" with subs:
#  | amount | period | txField  |*
#  |     10 |     1 | payerTid |
  # and many other fields

Scenario: A member makes a recurring donation
  When member ".ZZA" completes form "community/donate" with values:
  | amtChoice | amount | period | honor  | honored |*
  |        -1 |     10 |      M | memory | Jane Do |
	Then these "recurs":
	| id | created | from | to  | amount | period | purpose  |*
	|  1 | %today  | .ZZA | cgf |     10 |      M | donation |
  And transactions:
  | xid | created | amount | from | to   | purpose            | recursId |*
  |   1 | %today  |     10 | .ZZA | cgf  | donation (Monthly) |        1 |
  And we say "status": "gift successful"
  And these "honors":
  | created | uid  | honor  | honored |*
  | %today  | .ZZA | memory | Jane Do |
  And we notice "gift sent" to member ".ZZA" with subs:
  | amount | rewardAmount |*
  |    $10 |        $0.50 | 
  
Scenario: A member makes a new recurring donation
	Given these "recurs":
	| created   | from | to  | amount | period |*
	| %today-1d | .ZZA | cgf |     25 |      Y |
  When member ".ZZA" visits page "community/donate"
  Then we show "donation replaces" with:
  | period | amt |*
  | Yearly | $25 |

  When member ".ZZA" completes form "community/donate" with values:
  | amtChoice | amount | period | honor  | honored | share |*
  |        -1 |     10 |      M | memory | Jane Do |    10 |
  Then transactions:
  | xid | created | amount | from | to   | purpose            |*
  |   1 | %today  |     10 | .ZZA | cgf  | donation (Monthly) |
  And we say "status": "gift successful"
	And these "recurs":
	| created | from | to  | amount | period |*
	| %today  | .ZZA | cgf |     10 |      M |
  
Scenario: A company makes a recurring donation
  When member ".ZZC" completes form "community/donate" with values:
  | amtChoice | amount | period | honor  | honored |*
  |        -1 |     10 |      M | memory | Jane Do |
  Then transactions:
  | xid | created | amount | from | to   | purpose            |*
  |   1 | %today  |     10 | .ZZC | cgf  | donation (Monthly) |
  And we say "status": "gift successful"
	
Scenario: A member donates with insufficient funds
  When member ".ZZA" completes form "community/donate" with values:
  | amtChoice | amount | period | honor  | honored |*
  |        -1 |    200 |      X | memory | Jane Do |
  Then we say "status": "gift successful|gift transfer later"
  And invoices:
  | nvid | created | amount | from | to   | purpose  | flags | status   |*
  |    1 | %today  |    200 | .ZZA | cgf  | donation | gift  | approved |
  And these "honors":
  | created | uid  | honor  | honored |*
  | %today  | .ZZA | memory | Jane Do |
  And we tell "ctty" CO "gift" with subs:
  | amount | period |*
  |    200 |      X |
