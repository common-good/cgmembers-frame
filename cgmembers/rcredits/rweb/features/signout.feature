Feature: A user signs out of their Common Good account
AS a member
I WANT to sign out
SO I can go on to other things and protect the security of my account

Setup:
  Given members:
  | uid  | fullName | pass | email | flags  |*
  | .ZZA | Abe One  | a1   | a@    | member |
  And member is logged out

Scenario: A member signs out
  When member ".ZZA" visits page "signout"
  Then we redirect to "%PROMO_URL?region=NEW"

Scenario: A member times out and signs out automatically
  When member ".ZZA" visits page "signout/timedout"
  Then we redirect to "%PROMO_URL?region=NEW&timedout=1"