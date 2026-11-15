Feature: CGPay reset endpoints
AS the node cgpay app
I WANT PHP to send reset codes and set passwords for me
SO members can recover their accounts, with the codes and limits all kept on the node side.

Setup:
  Given members:
  | uid  | fullName | email | phone        | flags                                |*
  | .ZPB | Bea Two  | b@    | +14135550102 | member,confirmed,ok,carded,ided,debt |
  | .ZPC | Citre    | c@    | +14135550103 | member,confirmed,ok,carded,ided,debt,co |

# Rule: reset-deliver only sends; it does not decide anything

Scenario: The node app asks to email a reset code
  When cgpay posts "reset-deliver" with:
  | uid  | channel | code   |*
  | .ZPB | email   | 123456 |
  Then we reply "ok" with JSON:
  | channel |*
  | email   |
  And we email "reset-code" to member ".ZPB" with subs:
  | code   |*
  | 123456 |

Scenario: The node app asks to text a reset code but texting is not available
  When cgpay posts "reset-deliver" with:
  | uid  | channel | code   |*
  | .ZPB | sms     | 123456 |
  Then we reply "ok" with JSON:
  | channel |*
  | email   |
  And we email "reset-code" to member ".ZPB" with subs:
  | code   |*
  | 123456 |

Scenario: The node app asks to send a code to a company-only account
  When cgpay posts "reset-deliver" with:
  | uid  | channel | code   |*
  | .ZPC | email   | 123456 |
  Then we reply "ok" with JSON:
  | channel |*
  | %NUL    |

Scenario: The node app asks to send a code to a nonexistent account
  When cgpay posts "reset-deliver" with:
  | uid  | channel | code   |*
  | .ZZY | email   | 123456 |
  Then we reply "ok" with JSON:
  | channel |*
  | %NUL    |

Scenario: The node app asks to send a malformed code
  When cgpay posts "reset-deliver" with:
  | uid  | channel | code |*
  | .ZPB | email   | 12   |
  Then we reply "syntax"

# Rule: reset-complete sets the password only for a reset that node just verified

Scenario: The node app completes a verified reset
  Given these "reset_codes":
  | id | uid  | codeHash | expires | tries | channel | created | usedAt  |*
  | 7  | .ZPB | x        | %now+9n | 0     | email   | %now-1n | %now-1n |
  When cgpay posts "reset-complete" with:
  | resetId | uid  | newPassword |*
  | 7       | .ZPB | newpass99   |
  Then we reply "ok" with JSON:
  | ok   |*
  | true |
  And count "reset_codes" is 0
  And we email "password-changed" to member ".ZPB" with subs:
  | subject |*
  |         |

Scenario: The node app completes a reset that was never verified
  Given these "reset_codes":
  | id | uid  | codeHash | expires | tries | channel | created |*
  | 7  | .ZPB | x        | %now+9n | 0     | email   | %now-1n |
  When cgpay posts "reset-complete" with:
  | resetId | uid  | newPassword |*
  | 7       | .ZPB | newpass99   |
  Then we reply "forbidden"
  And count "reset_codes" is 1

Scenario: The node app completes a reset for the wrong account
  Given these "reset_codes":
  | id | uid  | codeHash | expires | tries | channel | created | usedAt  |*
  | 7  | .ZPB | x        | %now+9n | 0     | email   | %now-1n | %now-1n |
  When cgpay posts "reset-complete" with:
  | resetId | uid  | newPassword |*
  | 7       | .ZPC | newpass99   |
  Then we reply "forbidden"

Scenario: The node app completes a reset too late
  Given these "reset_codes":
  | id | uid  | codeHash | expires | tries | channel | created | usedAt   |*
  | 7  | .ZPB | x        | %now+9n | 0     | email   | %now-1n | %now-10n |
  When cgpay posts "reset-complete" with:
  | resetId | uid  | newPassword |*
  | 7       | .ZPB | newpass99   |
  Then we reply "forbidden"

Scenario: The node app completes a verified reset with a short password
  Given these "reset_codes":
  | id | uid  | codeHash | expires | tries | channel | created | usedAt  |*
  | 7  | .ZPB | x        | %now+9n | 0     | email   | %now-1n | %now-1n |
  When cgpay posts "reset-complete" with:
  | resetId | uid  | newPassword |*
  | 7       | .ZPB | short       |
  Then we reply "syntax"
  And count "reset_codes" is 1
