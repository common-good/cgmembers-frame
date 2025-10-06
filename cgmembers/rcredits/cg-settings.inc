<?php
/**
 * @file
 * Specialized settings for this rCredits server
 */

const R_SERVER_ID = 'NEW'; // qid header for this region's server (New England / Western MA)
// other 3rd letters: M=ME H=NH V=VT C=CT R=RI P=PA B=Boston
const ADMIN_NAME = 'William Spademan';
const SIGNATORY = 'William Spademan, President';
const R_ADMIN_EMAIL = 'info@CommonGood.earth'; // regional admin email
const R_ADMIN_PHONE = '+14136281957';
const R_REGION_EIN = '46-1821792';

date_default_timezone_set('America/New_York'); // server handles just one timezone
const R_COUNTRY_ID = US_COUNTRY_ID; // this region's country: United States
const R_STATE_ID = 1020; // Massachusetts

const R_XFEE_CARD = .03; // percentage fee for exchanges of USD for Common Good Credits using a credit/debit card
const R_XFEE_CHECK = 0; // flat fee for exchanges of USD for Common Good Credits with a check (was 3.00)
const MAX_AMOUNT = 100000; // transaction request size limit (some smaller limits also apply)
const R_FLOOR = 0; // how low can user's balance go, by default
const MIN_WEEKLY_BUMP = 1; // automatic weekly increase in negative minimum balance
const R_DAILY_HOUR = isDEV ? '0' : '1'; // what time (24-hour clock hour, one digit) to start intensive daily maintenance tasks
const R_WEEKLY_DAY = '1'; // what day of the week to do weekly tasks (Sunday), numbered according to ISO 8601
const R_MONTHLY_DAY = '01'; // what day of the month to do monthly tasks (2-digits)
const R_DEBT_FEE = 25; // fee for overdue reserve credit
const R_DEBT_PERCENT = 1.5; // percentage fee for overdue reserve credit
const R_DEBT_DAYS = 30; // how long an account can be negative
const R_STATEMENT_COST = 10; // how much member must pay (in rCredits dollars) for monthly and annual statements by mail
const R_INFLATION_RATE = .05; // annual USD inflation rate (but never less than zero) (extra big is okay)
const USDTX_DAYS = 5; // how many days it takes for a transfer to clear from bank to Common Good account
const USDTX_FAST = 200; // maximum amount a member can transfer overnight (instead of waiting USDTX days)
const DATE_FMT = 'MM/dd/yyyy'; // default date/time format (eg 06/15/2034)
const TIME_FMT = 'h:mma'; // default time format (eg 3:45pm)

const BANK_CHANGES = '7/1/2021, 10/1/2025'; // dates on which we changed banks (for operating and community fund accounts)
