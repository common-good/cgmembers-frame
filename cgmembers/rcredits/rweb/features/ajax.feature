Feature: Ajax
AS a Member
I WANT to see results quickly
SO I don't get bored and drop out.

# Unit tests for ajax operations

Setup:
  Given members:
  | uid  | fullName | city | state | flags           |*
  | .ZZA | Abe One  | Aton | AL    | ok,confirmed    |
  | .ZZB | Bea Two  | Bton | ME    | ok,confirmed    |
  | .ZZC | Cor Pub  | Cton | CA    | ok,confirmed,co |
  
Scenario: Ajax suggestWho
  When member ".ZZA" ajax "suggestWho" with:
  | data |  |**
  Then we show JSON ray:
  | uid     | nm                                      |*
  | .ZZA    | Abe One @ Aton, AL                      |
  | .ZZB    | Bea Two @ Bton, ME                      |
  | .AAB    | Common Good @ Ashfield, MA              |
  | !.AAA   | Common Good Western Mass @ Ashfield, MA |
  | .ZZC    | Cor Pub @ Cton, CA                      |
