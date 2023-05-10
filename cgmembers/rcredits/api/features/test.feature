Feature: Test
AS a developer
I WANT to put test data on the server and see it
SO I can set initial hypotheses and check expected results.

Setup:

Scenario: The app asks to initialize the test data
  When test op "initialize" with: ""
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
  And count "txs" is 0
  And we reply "ok" with JSON: "?"

Scenario: The app asks to initialize the test data when there are previous transactions
# without touching other data on the server
  Given members:
  | uid  | fullName | flags                                   | city |*
  | .ZPA | Abe One  | member,confirmed,ok,carded,ided,debt    | Aton |
  | .ZPI | Ida Nine | member,confirmed,ok,carded,ided,debt    | Iton |
  | .ZZA | Zeta1    | member,confirmed,ok,carded,ided,debt,co | Zton |
  | .ZZZ | Zeta2    | member,confirmed,ok,carded,ided,debt    | Zton |
  And these "txs":
  | uid1 | uid2 | amount | created |*
  | .ZPA | .ZPI | 12     | %now-1d |
  | .ZZA | .ZZZ | 999    | %now-1d |
  | .ZPI | .ZPA | 93     | %now-1d |
  Then count "txs" is 3

  When test op "initialize" with: ""
  Then count "txs" is 1
  And these "txs":
  | uid1 | uid2 | amount | created |*
  | .ZZA | .ZZZ | 999    | %now-1d |

Scenario: The app asks us for transaction data
  Given members:
  | uid  | fullName | flags                                   | city |*
  | .ZPA | Abe One  | member,confirmed,ok,carded,ided,debt    | Aton |
  | .ZPI | Ida Nine | member,confirmed,ok,carded,ided,debt    | Iton |
  | .ZZA | Zeta1    | member,confirmed,ok,carded,ided,debt,co | Zton |
  | .ZZZ | Zeta2    | member,confirmed,ok,carded,ided,debt    | Zton |
  And these "txs":
  | xid | uid1 | uid2 | amount | created |*
  | 1   | .ZPA | .ZPI | 12     | %now-1d |
  | 2   | .ZZA | .ZZZ | 999    | %now-1d |
  | 3   | .ZPI | .ZPA | 93     | %now-1d |
  When test op "rows" with:
  | fieldList | table |*
  | *         | txs   |
  Then we reply "ok" with JSON values:
  | xid | uid1 | uid2 | amt | created |*
  | 1   | .ZPA | .ZPI | 12  | %now-1d |
  | 3   | .ZPI | .ZPA | 93  | %now-1d |
