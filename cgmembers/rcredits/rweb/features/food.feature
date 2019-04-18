Feature: Food Fund
AS a member
I WANT to contribute to the Food Fund
SO I can help people who are struggling to afford healthy food.

Setup:
  Given members:
  | uid  | fullName  | floor | flags   |*
  | .ZZA | Abe One   |  -250 | ok,confirmed,nosearch,paper,debt |
  | .ZZB | Bea Two   |  -250 | ok,co,confirmed,weekly,secret |
  | .ZZF | Food Fund |     0 | ok,co,confirmed |
  
Scenario: A non-member visits the food page
  When member "?" visits page "settings/food/welcome=1"
  Then we show "Food Fund"

Scenario: A member signs in and sees link
  When member ".ZZA" visits page "summary"
  Then we show "Food Fund"

Scenario: A member visits the food page
  When member ".ZZA" visits page "settings/food"
  Then we show "Food Fund"

  When member ".ZZA" completes form "settings/food" with values:
  | food | amtChoice | amount | period |*
  |    3 |       100 |        |      M |
  Then members have:
  | uid  | food |*
  | .ZZA |  .03 |
  And these "recurs":
  | payer | payee | amount | period | ended |*
  | .ZZA  |  .ZZF |    100 |      M |     0 |
  And we say "status": "gift successful"