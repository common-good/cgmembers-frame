Feature: Test
AS a developer
I WANT to put test data on the server and see it
SO I can set initial hypotheses and check expected results.

Setup:
 
Scenario: The app asks to initialize the test data
  When test op "initialize" with ""
  Then members:
  | uid  | fullName | flags                                   | city |*
  | .ZPA | Abe One  | member,confirmed,ok,carded,ided,debt    | Aton |
  | .ZPB | Bea Two  | member,confirmed,ok,carded,ided,debt    | Bton |
  | .ZPC | Citre    | member,confirmed,ok,carded,ided,debt,co | Cton |
  | .ZPD | Dee Four | member,confirmed,ok,carded,ided,debt    | Dton |
  And these "u_relations":
  | main | agent | num | permission |*
  | .ZPC | .ZPA  |   1 | manage     |
  | .ZPC | .ZPB  |   2 | scan       |
  | .ZPG | .ZPF  |   1 | manage     |
  And member ".ZPA" has "card" steps done: "all"
