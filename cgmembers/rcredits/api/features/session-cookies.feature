Feature: Session Cookies
AS a member
I WANT to stay signed in when my browser still holds an old copy of my session cookie
SO a cookie set under an older scope (the host alone, a parent domain) does not log me out.

Setup:
  Given members:
  | uid  | fullName | flags                                |*
  | .ZPB | Bea Two  | member,confirmed,ok,carded,ided,debt |
  And session "live" belongs to member ".ZPB"

# Rule: The copy with a live session wins, and only the older scopes of that cookie name are expired.

Scenario: A stale cookie comes before the live one
  When the browser at "test.commongood.earth" sends session cookies: "stale, live"
  Then the session cookie is "live"
  And expired scopes are: "host .commongood.earth"

Scenario: Two stale cookies and no live one
  When the browser at "test.commongood.earth" sends session cookies: "stale, older"
  Then the session cookie is "stale"
  And expired scopes are: "host .commongood.earth"

Scenario: A cookie with another name is not part of it
  When the browser at "main.commongood.earth" sends session cookies: "stale, other=keep, live"
  Then the session cookie is "live"
  And expired scopes are: "host .commongood.earth"

# Rule: A single cookie is left alone, and a site with no parent domain expires only the host-only copy.

Scenario: A single cookie is left alone
  When the browser at "test.commongood.earth" sends session cookies: "live"
  Then the session cookie is "live"
  And expired scopes are: ""

Scenario: The apex site has no parent domain to expire
  When the browser at "commongood.earth" sends session cookies: "stale, live"
  Then the session cookie is "live"
  And expired scopes are: "host"
