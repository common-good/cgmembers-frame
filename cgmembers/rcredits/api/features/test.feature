Feature: Test
AS a developer
I WANT to put test data on the server and see it
SO I can set initial hypotheses and check expected results.

Setup:
 
Scenario: The app asks to initialize the test data
  When test op "initialize" with ""
  Then members:
  | uid  | fullName | flags                                   | city |*
  | .ZZA | Abe One  | member,confirmed,ok,carded,ided,debt    | Aton |
  | .ZZB | Bea Two  | member,confirmed,ok,carded,ided,debt    | Bton |
  | .ZZC | Citre    | member,confirmed,ok,carded,ided,debt,co | Cton |
  | .ZZD | Dee Four | member,confirmed,ok,carded,ided,debt    | Dton |
  And these "u_relations":
  | main | agent | num | permission |*
  | .ZZC | .ZZA  |   1 | manage     |
  | .ZZC | .ZZB  |   2 | scan       |
  | .ZZG | .ZZF  |   1 | manage     |
  And member ".ZZA" has "card" steps done: "all"
