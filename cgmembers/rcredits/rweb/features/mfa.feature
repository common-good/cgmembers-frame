Feature: A user signs in to their Common Good account with multi-factor authentication
AS a member
I WANT to sign in to my Common Good account extra securely
SO I can view or change settings, view or handle past transactions, and/or pay or charge another account, without getting hacked

Setup:
  Given members:
  | uid  | fullName | pass | email | flags      |*
  | .ZZA | Abe One  | a1   | a@    | member,mfa |
  And member is logged out

Scenario: A member visits the member site
  When member "?" visits page "signin"
  Then we show "Welcome to %PROJECT" with:
  | Account ID | account ID or email |
  | Password   | Reset password |
  |~promo      | Not yet a member? |

Scenario: A member signs in with account ID on the member site with MFA
  Given next random code is "987654"
  And var "mfa" encrypts:
  | uid  | nonce     |*
  | .ZZA | %nextCode |
  When member "?" completes "signin" with:
  | qid  | pass |*
  | .ZZA | a1   |
  Then we redirect to "signin/mfa=%mfa"
  And we show "Confirm" with:
  | Passcode |
  And member is logged out
  
  When member "?" completes "signin/mfa=%mfa" with:
  | uid  | mfa  | then | gotNonce |*
  | .ZZA | %mfa |      | 987654   |
  Then member ".ZZA" is logged in
  And we show "You: Abe One"

Scenario: A member types the wrong nonce
  Given next random code is "987654"
  And var "mfa" encrypts:
  | uid  | nonce     |*
  | .ZZA | %nextCode |
  When member "?" completes "signin" with:
  | qid  | pass |*
  | .ZZA | a1   |
  And member "?" completes "signin/mfa=%mfa" with:
  | uid  | mfa  | then | gotNonce |*
  | .ZZA | %mfa |      | 999999   |
  Then member is logged out
  And we say "error": "bad nonce"
