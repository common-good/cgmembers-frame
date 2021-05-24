Feature: Company Information
AS member with more than one Common Good account to manage
I WANT to change to a different account
SO I can manage that one instead.

Setup:
  Given members:
  | uid  | fullName | flags     |*
  | .ZZA | Abe One  | ok        |
  | .ZZB | Bea Two  | ok,cAdmin |
  | .ZZC | Our Pub  | co,ok     |
  And relations:
  | main | agent | num | permission |*
  | .ZZC | .ZZA  |   1 | manage     |
  
Scenario: A member clicks the profile photo
  When member "A" visits page "accounts"
  Then we show "" with:
  | Abe One |
  | Our Pub |

Scenario: A member changes account
  When member "A" visits page "change-account/acct=NEWZZC"
  Then we show "You: Our Pub"

Scenario: A member changes account to go to a different page
  When member "A" visits page "change-account/acct=NEWZZC&page=settings,contact"
  Then we show "Contact Info" with:
  | Company Name | Our Pub       |
  | Email        | c@example.com |

Scenario: A community admin clicks the profile photo
  When member "B" visits page "accounts"
  Then we show "Switch to account"

Scenario: A community admin changes account
  When member "B" visits page "change-account/acct=NEWZZC"
  Then we show "Account Summary" with:
  | ID | NEWZZC |

Scenario: A superadmin clicks the profile photo
  When member "1" visits page "accounts"
  Then we show "Switch to account"

Scenario: A superadmin changes account
  When member "1" visits page "change-account/acct=NEWZZC"
  Then we show "Account Summary" with:
  | ID | NEWZZC |

Scenario: A member tries to change to an account without permission
  When member "A" visits page "change-account/acct=NEWZZB"
  Then we tell admin "HACK attempt: change to illegal" with subs: ""