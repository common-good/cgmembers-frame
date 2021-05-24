Feature: FeatureName
AS a StakeholderType
I WANT to DoWhat
SO IntendedOutcome

Setup:
  Given members:
  | uid  | fullName | flags   |*
  | .ZZA | Abe One  | ok,confirmed      |
  | .ZZB | Bea Two  | ok,confirmed      |
  | .ZZC | Cor Pub  | ok,confirmed,co   |
  
Scenario: Synopsis of scenario
  Given this
  When that
  Then result1
  And result2:
  | column1 | column2 |*
  | this    | that    |
  And result3:
  | column1 | this |**
  | column2 | that |
  And result4:
  | this,   | that,     |
  | and     | the other |
  
