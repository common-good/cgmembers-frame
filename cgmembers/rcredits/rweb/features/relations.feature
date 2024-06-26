Feature: Relations
AS a member
I WANT to manage my account's relations with other rCredits accounts
SO I can buy and sell stuff on behalf of other accounts, and they on mine.

Setup:
  Given members:
  | uid  | fullName   | acctType    | flags      |*
  | .ZZA | Abe One    | personal    | ok         |
  | .ZZB | Bea Two    | personal    | ok         |
  | .ZZC | Corner Pub | corporation | ok,co      |
  | .ZZD | Dee Four   | personal    | ok         |

Scenario: Member has an employee, confirmed
  Given these "u_relations":
  | main | agent | permission   | employee | owner | draw |*
  | .ZZA | .ZZD  | sell         | 1          | 1       | 0    |
  When member ".ZZA" visits page "settings/relations"
  Then we show "Relations" with:
  | other      | Draw | Employee | Family | Permission |      |
  | Dee Four   | No   | Yes      | Yes    | %can_sell  | %DEL |

Scenario: Member has an employee, unconfirmed
  Given these "u_relations":
  | main | agent | permission | employee | owner |*
  | .ZZA | .ZZD  | refund     | 1          | 0       |
  When member ".ZZA" visits page "settings/relations"
  Then we show "Relations" with:
  | other      | Employee | Family | Permission  |
  | Dee Four   | Yes          | No      | %can_refund |

Scenario: Member has a relation with a contractor
  Given these "u_relations":
  | main | agent | permission | employee | owner |*
  | .ZZA | .ZZB  | buy        | 0          | 0        |
  When member ".ZZA" visits page "settings/relations"
  Then we show "Relations" with:
  | other      | Employee | Family | Permission |
  | Bea Two    | No           | No      | %can_buy   |
  
Scenario: Member has an employee, claimed
  Given these "u_relations":
  | main | agent | permission   | employee | owner |*
  | .ZZA | .ZZD  | sell         | 0          | 0       |
  When member ".ZZA" visits page "settings/relations"
  Then we show "Relations" with:
  | other      | Employee | Family | Permission |
  | Dee Four   | No           | No      | %can_sell  |
  
Scenario: Employee can only read
  Given these "u_relations":
  | main | agent | permission | employee | owner |*
  | .ZZA | .ZZD  | read       | 1          | 1       |
  When member ".ZZA" visits page "settings/relations"
  Then we show "Relations" with:
  | other      | Employee | Family | Permission |
  | Dee Four   | Yes          | Yes     | %can_read  |
  
Scenario: Member has an employer
  Given these "u_relations":
  | main | agent | permission   | employee | owner |*
  | .ZZB | .ZZA  | sell         | 1          | 1        |
  When member ".ZZA" visits page "settings/relations"
  Then we show "Relations" with:
  | other      | Employee | Family | Permission |
  And without:
  | Bea Two |
  
Scenario: Member has access to employee account
  Given these "u_relations":
  | main | agent | permission | employee | owner |*
  | .ZZA | .ZZD  |            | 1          | 1        |
  | .ZZD | .ZZA  | sell       | 0          | 0        |
  When member ".ZZA" visits page "settings/relations"
  Then we show "Relations" with:
  | other      | Employee | Family | Permission |
  | Dee Four   | Yes          | Yes     | %can_none  |
  When member ".ZZD" visits page "settings/relations"
  Then we show "Relations" with:
  | other      | Employee | Family | Permission |
  | Abe One    | No           | No      | %can_sell  |

Scenario: Member company has relations
  Given these "u_relations":
  | main | agent | num | permission | employee | owner |*
  | .ZZC | .ZZA  |   1 | manage     | 1        | 1     |
  When member "C:A" visits page "settings/relations"
  Then we show "Relations" with:
  | other   | Employee | Owns | Permission     |
  | Abe One | Yes      | Yes  | manage account |
#  | Abe One | Yes          | Yes  | manage account | request Common Good Card |
  And without:
  | %DEL |
  When member ".ZZA" visits page "settings/relations"
  Then we show "Relations" with:
  | other      | Employee | Family | Permission   |
  And without:
  | Corner Pub |

Scenario: It's complicated
  Given these "u_relations":
  | main | agent | num | permission | employee | owner |*
  | .ZZA | .ZZB  |   0 | sell       | 1        | 1       |
  | .ZZA | .ZZC  |   0 | buy        | 0        | 0       |
  | .ZZA | .ZZD  |   0 | manage     | 1        | 1       |
  | .ZZB | .ZZA  |   0 |            | 1        | 0       |
  | .ZZC | .ZZA  |   1 | sell       | 1        | 0       |
  | .ZZC | .ZZD  |   1 | manage     | 1        | 0       |
  | .ZZD | .ZZA  |   0 |            | 0        | 0       |
  When member ".ZZA" visits page "settings/relations"
  Then we show "Relations" with:
  | other      | Employee | Family | Permission  |      |
  | Bea Two    | Yes      | Yes    | %can_sell   | %DEL |
  | Corner Pub | --       | No     | --          | %DEL |
  | Dee Four   | Yes      | Yes    | %can_manage | %DEL |
  When member ".ZZB" visits page "settings/relations"
  Then we show "Relations" with:
  | other   | Employee | Family | Permission |
  | Abe One | Yes      | No     | %can_none  |
  When member ".ZZD" visits page "settings/relations"
  Then we show "Relations" with:
  | other   | Employee | Family | Permission |
  | Abe One | No       | No     | %can_none  |
  And with:
  |~Header  |
  | Family |
  When member "C:A" visits page "settings/relations"
  Then we say "error": "no page permission" with subs:
  | page | Corner Pub: Relations |**
  
  When member "C:D" visits page "settings/relations"
  Then we show "Relations" with:
  | other   | Employee | Owns | Permission |
  | Abe One | Yes      | No   | %can_sell  |
#  | Abe One | Yes          | No   | %can_sell | request Cashier Card |

Scenario: A member adds a relation
# This test fails (but works fine). Dunno why.
  When member ".ZZA" completes relations form with values:
  | newPerson |*
  | Bea Two   |
  Then we say "status": "report new relation" with subs:
  | who     |*
  | Bea Two |
  And we show "Relations" with:
  | other      | Draw | Employee | Family | Permission |
  | Bea Two    | No   | No       | No     | %can_read  |

Scenario: A member tries to add a relation with self
  When member ".ZZA" completes relations form with values:
  | newPerson |*
  | Abe One   |
  Then we say "error": "self-relation"

Scenario: A member tries to add a relation again
  Given these "u_relations":
  | main | agent | permission | employee | owner |*
  | .ZZA | .ZZB  | sell       | 1        | 1       |
  When member ".ZZA" completes relations form with values:
  | newPerson |*
  | Bea Two   |
  Then we say "error": "already related"
  