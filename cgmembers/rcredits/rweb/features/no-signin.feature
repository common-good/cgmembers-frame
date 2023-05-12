Feature: No Sign-in
AS a Member
I WANT to complete some operation on my Common Good account by clicking a link, without signing in
SO I don't have to hunt for my password or struggle with my bad typing or low-threshold annoyance

Setup:
  Given members:
  | uid  | fullName | flags   |*
  | .ZZA | Abe One  | ok,confirmed      |
  | .ZZB | Bea Two  | ok,confirmed      |
  | .ZZC | Cor Pub  | ok,confirmed,co   |
  
Scenario: A member donates from an email link
  Given member is signed out
  And var "CODE" encrypts:
  | qid | NEWZZA |**
  When member "?" visits page "do/doDonate~%CODE"
  Then we show "Donate to %PROJECT" with:
  | Donation: |
  | When:     |
  | Honoring: |
  | Donate    |
  And without:
  | Name: |
  | Phone: |
  | Email: |
  | Country: |
  | Postal Code: |
