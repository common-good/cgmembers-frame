<?php
/**
 * @file
 * Descriptions of flag bits.
 */

$flagHelp = array(
  'member' => t('All account opening steps have been completed.'),
  'confirmed' => t('Whoever invited the member has confirmed the invitation OR member bought something locally in-person.'),
  'ok' => t('Account is active -- approved for transactions.'),
  'carded' => t('A %PROJECT card and welcome letter has been mailed to the member.'),
  'ided' => t('A cashier has verified the member\'s government-issued photo ID.'),
  'refill' => t('The member refills the account automatically from their bank account.'),
  'co' => t('The account is a company, organization, club, or fund -- not a person.'),
  'draws' => t('This account automatically draws funds from one or more others.'),
  'roundup' => t('Round payments up to nearest dollar, donate the cents to the community fund and the common good.'),
  'nonudge' => t('Don\'t send reminder emails.'),
  'nosearch' => t('Member wants to be not findable by other members, except by phone or email.'),
  'depends' => t('Account is a dependent account (individual or business). Can\'t change settings. FederalId is sponsor\'s.'),
  'bankOk' => t('The member\'s connected bank account has been verified.'),
  'contactable' => t('It\'s okay for members to contact this member about their calling.'),
  'mfa' => t('Use multi-factor authentication.'),
// u14
  'partner' => t('The account is a signup partner.'),
  'paper' => t('Will not accept electronic statements.'),
  'secret' => t('Don\'t let merchants tell the member their balance on request.'),
  'underage' => t('Has not yet reached the legal age to be allowed a credit line.'),
  'debt' => t('Member chose to permit sub-zero balances.'),
  'reinvest' => t('Member chose to automatically reinvest investment club dividends and inflation adjustments.'),
  'savings' => t('NYI The connected bank account is a savings or money market account.'),
  'cashoutW' => t('Cash this account out weekly, leaving only its average weekly gross in the account.'),
  'cashoutM' => t('Cash this account out monthly, leaving only its average weekly gross in the account.'),
  'iclubq' => t('The person is interested in an investment club.'),
// u26
// u27
// u28
  'influencer' => t('The person has significant influence over %PROJECT (board member, former board member, or Executive Director) or is a family member of such a person.'),
//  'icAdmin' => t('The account is an Investment Club administrator.'),
//  'oAdmin' => t('The account is an administrator of the Offers, Needs, & Tips board in a community.'),
//  'admin2' => t('The account is an overall assistant administrator or bank partner or government regulator (the Admin2 has read-only permission on all accounts, except with permission to add notes).'),
//  'cAdmin2' => t('The account is a community sub-administrator (the cAdmin2 has read-only permission on all accounts, except with permission to add notes).'),
//  'cAdmin' => t('The account is a community administrator.'),
  'admin' => t('The account is a system administrator -- specific permissions are set separately.'),
  
  'cgc' => t('Account is an effectively autonomous Common Good Community.'),
  'up' => t('Community is operational (up).'),
  'invites' => t('Community requires invitations.'),
  'rewardy' => t('Community issues rewards explicitly.'),
  'noneg' => t('Community does not reduce a member\'s credit line on overdrafts.'),
);
