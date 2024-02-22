-- phpMyAdmin SQL Dump
-- version 5.2.1
-- https://www.phpmyadmin.net/
--
-- Host: 127.0.0.1:3307
-- Generation Time: Feb 02, 2024 at 04:27 PM
-- Server version: 11.2.2-MariaDB
-- PHP Version: 8.2.13

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `cg`
--
CREATE DATABASE IF NOT EXISTS `cg` DEFAULT CHARACTER SET latin1 COLLATE latin1_swedish_ci;
USE `cg`;

DELIMITER $$
--
-- Functions
--
CREATE DEFINER=`cg_user`@`localhost` FUNCTION `perYear` (`period` ENUM('once','day','week','month','quarter','year','forever'), `periods` INT(11)) RETURNS TINYINT(4)  RETURN (CASE period
        WHEN 'forever' THEN 0
        WHEN 'year' THEN 1
        WHEN 'quarter' THEN 4
        WHEN 'month' THEN 12
        WHEN 'week' THEN 365.25/7
        WHEN 'day' THEN 365.25
        WHEN 'once' THEN 0
        ELSE -1
      END) / periods$$

DELIMITER ;

-- --------------------------------------------------------

--
-- Table structure for table `admins`
--

CREATE TABLE `admins` (
  `id` int(11) NOT NULL,
  `uid` bigint(20) DEFAULT NULL COMMENT 'related account record ID',
  `vKeyE` longblob DEFAULT NULL COMMENT 'very secret private key (vKey) encrypted with a password specific to this account',
  `can` bigint(20) DEFAULT NULL COMMENT 'bit array of permissions for this admin'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_general_ci COMMENT='permissions for each admin';

-- --------------------------------------------------------

--
-- Stand-in structure for view `ancestors`
-- (See below for the actual view)
--
CREATE TABLE `ancestors` (
`base` int(11)
,`baseIndustry` varchar(255)
,`ancestor` int(11)
,`ancestorIndustry` varchar(255)
);

-- --------------------------------------------------------

--
-- Table structure for table `block`
--

CREATE TABLE `block` (
  `bid` int(11) NOT NULL COMMENT 'Primary Key: Unique block ID.',
  `module` varchar(64) NOT NULL DEFAULT '' COMMENT 'The module from which the block originates; for example, ’user’ for the Who’s Online block, and ’block’ for any custom blocks.',
  `delta` varchar(32) NOT NULL DEFAULT '0' COMMENT 'Unique ID for block within a module.',
  `theme` varchar(64) NOT NULL DEFAULT '' COMMENT 'The theme under which the block settings apply.',
  `status` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'Block enabled status. (1 = enabled, 0 = disabled)',
  `weight` int(11) NOT NULL DEFAULT 0 COMMENT 'Block weight within region.',
  `region` varchar(64) NOT NULL DEFAULT '' COMMENT 'Theme region within which the block is set.',
  `custom` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'Flag to indicate how users may control visibility of the block. (0 = Users cannot control, 1 = On by default, but can be hidden, 2 = Hidden by default, but can be shown)',
  `visibility` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'Flag to indicate how to show blocks on pages. (0 = Show on all pages except listed pages, 1 = Show only on listed pages, 2 = Use custom PHP code to determine visibility)',
  `pages` mediumtext NOT NULL COMMENT 'Contents of the "Pages" block; contains either a list of paths on which to include/exclude the block or PHP code, depending on "visibility" setting.',
  `title` varchar(255) NOT NULL DEFAULT '' COMMENT 'Custom title for the block. (Empty string will use block default title, <none> will remove the title, text will cause block to use specified title.)',
  `cache` tinyint(4) NOT NULL DEFAULT 1 COMMENT 'Binary flag to indicate block cache mode. (-2: Custom cache, -1: Do not cache, 1: Cache per role, 2: Cache per user, 4: Cache per page, 8: Block cache global) See DRUPAL_CACHE_* constants in ../includes/common.inc for more detailed information.'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Stores block settings, such as region and visibility...' ROW_FORMAT=DYNAMIC;

--
-- Dumping data for table `block`
--

INSERT INTO `block` VALUES(423, 'rweb', 'accounts', 'rcredits', 1, 0, 'accounts', 0, 0, '', '', 1);
INSERT INTO `block` VALUES(424, 'rweb', 'footer', 'rcredits', 1, 0, 'footer', 0, 0, '', '', -1);
INSERT INTO `block` VALUES(425, 'rweb', 'accounts', 'seven', 0, 0, '-1', 0, 0, '', '', 1);
INSERT INTO `block` VALUES(426, 'rweb', 'footer', 'seven', 0, 0, '-1', 0, 0, '', '', -1);

-- --------------------------------------------------------

--
-- Table structure for table `cache`
--

CREATE TABLE `cache` (
  `cid` varchar(255) NOT NULL DEFAULT '' COMMENT 'Primary Key: Unique cache ID.',
  `data` longblob DEFAULT NULL COMMENT 'A collection of data to cache.',
  `expire` int(11) NOT NULL DEFAULT 0 COMMENT 'A Unix timestamp indicating when the cache entry should expire, or 0 for never.',
  `created` int(11) NOT NULL DEFAULT 0 COMMENT 'A Unix timestamp indicating when the cache entry was created.',
  `serialized` smallint(6) NOT NULL DEFAULT 0 COMMENT 'A flag to indicate whether content is serialized (1) or not (0).'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Generic cache table for caching things not separated out...' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `cache_bootstrap`
--

CREATE TABLE `cache_bootstrap` (
  `cid` varchar(255) NOT NULL DEFAULT '' COMMENT 'Primary Key: Unique cache ID.',
  `data` longblob DEFAULT NULL COMMENT 'A collection of data to cache.',
  `expire` int(11) NOT NULL DEFAULT 0 COMMENT 'A Unix timestamp indicating when the cache entry should expire, or 0 for never.',
  `created` int(11) NOT NULL DEFAULT 0 COMMENT 'A Unix timestamp indicating when the cache entry was created.',
  `serialized` smallint(6) NOT NULL DEFAULT 0 COMMENT 'A flag to indicate whether content is serialized (1) or not (0).'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Cache table for data required to bootstrap Drupal, may be...' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `cache_form`
--

CREATE TABLE `cache_form` (
  `cid` varchar(255) NOT NULL DEFAULT '' COMMENT 'Primary Key: Unique cache ID.',
  `data` longblob DEFAULT NULL COMMENT 'A collection of data to cache.',
  `expire` int(11) NOT NULL DEFAULT 0 COMMENT 'A Unix timestamp indicating when the cache entry should expire, or 0 for never.',
  `created` int(11) NOT NULL DEFAULT 0 COMMENT 'A Unix timestamp indicating when the cache entry was created.',
  `serialized` smallint(6) NOT NULL DEFAULT 0 COMMENT 'A flag to indicate whether content is serialized (1) or not (0).'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Cache table for the form system to store recently built...' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `cache_menu`
--

CREATE TABLE `cache_menu` (
  `cid` varchar(255) NOT NULL DEFAULT '' COMMENT 'Primary Key: Unique cache ID.',
  `data` longblob DEFAULT NULL COMMENT 'A collection of data to cache.',
  `expire` int(11) NOT NULL DEFAULT 0 COMMENT 'A Unix timestamp indicating when the cache entry should expire, or 0 for never.',
  `created` int(11) NOT NULL DEFAULT 0 COMMENT 'A Unix timestamp indicating when the cache entry was created.',
  `serialized` smallint(6) NOT NULL DEFAULT 0 COMMENT 'A flag to indicate whether content is serialized (1) or not (0).'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Cache table for the menu system to store router...' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `cu_folders`
--

CREATE TABLE `cu_folders` (
  `id` bigint(20) NOT NULL COMMENT 'record ID',
  `name` varchar(255) DEFAULT NULL COMMENT 'folder name'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_general_ci COMMENT='groups of lists';

-- --------------------------------------------------------

--
-- Table structure for table `cu_lists`
--

CREATE TABLE `cu_lists` (
  `id` bigint(20) NOT NULL COMMENT 'record ID',
  `name` varchar(255) DEFAULT NULL COMMENT 'list name',
  `folder` bigint(20) DEFAULT NULL COMMENT 'folder record ID, if any',
  `space` bigint(20) DEFAULT NULL COMMENT 'space record ID, if not in any folder'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_general_ci COMMENT='lists of tasks';

-- --------------------------------------------------------

--
-- Table structure for table `cu_members`
--

CREATE TABLE `cu_members` (
  `id` bigint(20) NOT NULL COMMENT 'record ID',
  `uid` bigint(20) DEFAULT NULL COMMENT 'account record ID, if any',
  `name` varchar(255) DEFAULT NULL COMMENT 'team member name',
  `nick` varchar(5) DEFAULT NULL COMMENT 'team member nickname'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_general_ci COMMENT='clickup members and guests on the Common Good team';

-- --------------------------------------------------------

--
-- Table structure for table `cu_spaces`
--

CREATE TABLE `cu_spaces` (
  `id` bigint(20) NOT NULL COMMENT 'record ID',
  `name` varchar(255) DEFAULT NULL COMMENT 'space name'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_general_ci COMMENT='top level categories';

-- --------------------------------------------------------

--
-- Table structure for table `cu_tasks`
--

CREATE TABLE `cu_tasks` (
  `id` varchar(255) NOT NULL COMMENT 'record ID',
  `name` varchar(255) DEFAULT NULL COMMENT 'task name',
  `parent` varchar(255) DEFAULT NULL COMMENT 'parent task, if this is a subtask',
  `list` bigint(20) DEFAULT NULL COMMENT 'list record ID',
  `status` varchar(255) DEFAULT NULL COMMENT 'task status',
  `priority` varchar(255) DEFAULT NULL COMMENT 'task priority',
  `class` varchar(255) DEFAULT NULL COMMENT 'category',
  `tags` varchar(255) DEFAULT NULL COMMENT 'comma-delimted list of tags for this task',
  `estimate` bigint(20) DEFAULT NULL COMMENT 'estimated time to complete this task',
  `cap` bigint(20) DEFAULT NULL COMMENT 'maximum time to complete this task',
  `spent` bigint(20) DEFAULT NULL COMMENT 'time spent on this task',
  `closed` bigint(20) DEFAULT NULL COMMENT 'date/time task was closed'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_general_ci COMMENT='things to be done';

-- --------------------------------------------------------

--
-- Table structure for table `cu_times`
--

CREATE TABLE `cu_times` (
  `id` bigint(20) NOT NULL COMMENT 'record ID',
  `task` varchar(255) DEFAULT NULL COMMENT 'task on which time was spent',
  `member` varchar(255) DEFAULT NULL COMMENT 'team member record ID',
  `start` bigint(20) DEFAULT NULL COMMENT 'date/time started',
  `stop` bigint(20) DEFAULT NULL COMMENT 'date/time stopped'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_general_ci COMMENT='amount of time spent on tasks';

-- --------------------------------------------------------

--
-- Stand-in structure for view `descendants`
-- (See below for the actual view)
--
CREATE TABLE `descendants` (
`base` int(11)
,`baseIndustry` varchar(255)
,`descendant` int(11)
,`descendantIndustry` varchar(255)
);

-- --------------------------------------------------------

--
-- Table structure for table `flood`
--

CREATE TABLE `flood` (
  `fid` int(11) NOT NULL COMMENT 'Unique flood event ID.',
  `event` varchar(64) NOT NULL DEFAULT '' COMMENT 'Name of event (e.g. contact).',
  `identifier` varchar(128) NOT NULL DEFAULT '' COMMENT 'Identifier of the visitor, such as an IP address or hostname.',
  `timestamp` int(11) NOT NULL DEFAULT 0 COMMENT 'Timestamp of the event.',
  `expiration` int(11) NOT NULL DEFAULT 0 COMMENT 'Expiration timestamp. Expired events are purged on cron run.'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Flood controls the threshold of events, such as the...' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `legacy_x_invoices`
--

CREATE TABLE `legacy_x_invoices` (
  `deleted` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime record was deleted',
  `nvid` bigint(20) NOT NULL COMMENT 'the unique invoice ID',
  `status` int(11) NOT NULL DEFAULT -1 COMMENT 'transaction record ID or status (approved, pending, denied, or paid)',
  `amount` decimal(11,2) DEFAULT NULL COMMENT 'amount to charge',
  `payer` bigint(20) DEFAULT NULL COMMENT 'user id of the payer',
  `payee` bigint(20) DEFAULT NULL COMMENT 'user id of the payee',
  `goods` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'is this an invoice for real goods and services?',
  `purpose` longtext DEFAULT NULL COMMENT 'payee''s description',
  `flags` bigint(20) UNSIGNED NOT NULL DEFAULT 0 COMMENT 'boolean characteristics and state flags',
  `data` longtext DEFAULT NULL COMMENT 'miscellaneous non-searchable data (serialized array)',
  `reversesXid` bigint(20) DEFAULT NULL COMMENT 'xid of the transaction this invoice reverses (if any)',
  `recursId` bigint(20) DEFAULT NULL COMMENT 'related record in tx_rules, for recurring charge (or reversed payment)',
  `created` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime invoice was created'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Record of all rCredits invoices in the region' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `legacy_x_txs`
--

CREATE TABLE `legacy_x_txs` (
  `deleted` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime record was deleted',
  `xid` bigint(20) NOT NULL COMMENT 'the unique transaction ID',
  `serial` int(11) DEFAULT NULL COMMENT 'serial number of related transactions (=xid of first transaction in the group)',
  `type` tinyint(4) DEFAULT NULL COMMENT 'transaction type (transfer, rebate, etc.)',
  `goods` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'is this transfer an exchange for real goods and services?',
  `amount` decimal(11,2) DEFAULT NULL COMMENT 'amount transferred',
  `payer` bigint(20) DEFAULT NULL COMMENT 'user id of the payer',
  `payee` bigint(20) DEFAULT NULL COMMENT 'user id of the payee',
  `payerAgent` bigint(20) DEFAULT NULL COMMENT 'user id of payer''s agent (who approved this transaction for the payer)',
  `payeeAgent` bigint(20) DEFAULT NULL COMMENT 'user id of payee''s agent (who approved this transaction for the payee)',
  `payerFor` varchar(255) DEFAULT NULL COMMENT 'payer''s description',
  `payeeFor` varchar(255) DEFAULT NULL COMMENT 'payee''s description',
  `payerTid` int(11) DEFAULT NULL COMMENT 'payer''s transaction ID',
  `payeeTid` int(11) DEFAULT NULL COMMENT 'payee''s transaction ID',
  `data` longtext DEFAULT NULL COMMENT 'miscellaneous non-searchable data (serialized array)',
  `flags` bigint(20) UNSIGNED NOT NULL DEFAULT 0 COMMENT 'boolean characteristics and state flags',
  `channel` tinyint(4) DEFAULT NULL COMMENT 'through what medium was the transaction entered',
  `box` int(11) NOT NULL DEFAULT 0 COMMENT 'on what machine was the transaction entered',
  `created` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime transaction was created',
  `risk` float DEFAULT NULL COMMENT 'suspiciousness rating',
  `risks` bigint(20) UNSIGNED NOT NULL DEFAULT 0 COMMENT 'list of risk factors'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Record of all rCredits transactions in the region' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `menu_links`
--

CREATE TABLE `menu_links` (
  `menu_name` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT 'The menu name. All links with the same menu name (such as ’navigation’) are part of the same menu.',
  `mlid` int(10) UNSIGNED NOT NULL COMMENT 'The menu link ID (mlid) is the integer primary key.',
  `plid` int(10) UNSIGNED NOT NULL DEFAULT 0 COMMENT 'The parent link ID (plid) is the mlid of the link above in the hierarchy, or zero if the link is at the top level in its menu.',
  `link_path` varchar(191) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT 'The Drupal path or external path this link points to.',
  `router_path` varchar(191) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT 'For links corresponding to a Drupal path (external = 0), this connects the link to a menu_router.path for joins.',
  `link_title` varchar(191) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '' COMMENT 'The text displayed for the link, which may be modified by a title callback stored in menu_router.',
  `options` blob DEFAULT NULL COMMENT 'A serialized array of options to be passed to the url() or l() function, such as a query string or HTML attributes.',
  `module` varchar(191) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT 'system' COMMENT 'The name of the module that generated this link.',
  `hidden` smallint(6) NOT NULL DEFAULT 0 COMMENT 'A flag for whether the link should be rendered in menus. (1 = a disabled menu item that may be shown on admin screens, -1 = a menu callback, 0 = a normal, visible link)',
  `external` smallint(6) NOT NULL DEFAULT 0 COMMENT 'A flag to indicate if the link points to a full URL starting with a protocol, like http:// (1 = external, 0 = internal).',
  `has_children` smallint(6) NOT NULL DEFAULT 0 COMMENT 'Flag indicating whether any links have this link as a parent (1 = children exist, 0 = no children).',
  `expanded` smallint(6) NOT NULL DEFAULT 0 COMMENT 'Flag for whether this link should be rendered as expanded in menus - expanded links always have their child links displayed, instead of only when the link is in the active trail (1 = expanded, 0 = not expanded)',
  `weight` int(11) NOT NULL DEFAULT 0 COMMENT 'Link weight among links in the same menu at the same depth.',
  `depth` smallint(6) NOT NULL DEFAULT 0 COMMENT 'The depth relative to the top level. A link with plid == 0 will have depth == 1.',
  `customized` smallint(6) NOT NULL DEFAULT 0 COMMENT 'A flag to indicate that the user has manually created or edited the link (1 = customized, 0 = not customized).',
  `p1` int(10) UNSIGNED NOT NULL DEFAULT 0 COMMENT 'The first mlid in the materialized path. If N = depth, then pN must equal the mlid. If depth > 1 then p(N-1) must equal the plid. All pX where X > depth must equal zero. The columns p1 .. p9 are also called the parents.',
  `p2` int(10) UNSIGNED NOT NULL DEFAULT 0 COMMENT 'The second mlid in the materialized path. See p1.',
  `p3` int(10) UNSIGNED NOT NULL DEFAULT 0 COMMENT 'The third mlid in the materialized path. See p1.',
  `p4` int(10) UNSIGNED NOT NULL DEFAULT 0 COMMENT 'The fourth mlid in the materialized path. See p1.',
  `p5` int(10) UNSIGNED NOT NULL DEFAULT 0 COMMENT 'The fifth mlid in the materialized path. See p1.',
  `p6` int(10) UNSIGNED NOT NULL DEFAULT 0 COMMENT 'The sixth mlid in the materialized path. See p1.',
  `p7` int(10) UNSIGNED NOT NULL DEFAULT 0 COMMENT 'The seventh mlid in the materialized path. See p1.',
  `p8` int(10) UNSIGNED NOT NULL DEFAULT 0 COMMENT 'The eighth mlid in the materialized path. See p1.',
  `p9` int(10) UNSIGNED NOT NULL DEFAULT 0 COMMENT 'The ninth mlid in the materialized path. See p1.',
  `updated` smallint(6) NOT NULL DEFAULT 0 COMMENT 'Flag that indicates that this link was generated during the update from Drupal 5.'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_general_ci COMMENT='Contains the individual links within a menu.' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `menu_router`
--

CREATE TABLE `menu_router` (
  `path` varchar(255) NOT NULL DEFAULT '' COMMENT 'Primary Key: the Drupal path this entry describes',
  `load_functions` blob NOT NULL COMMENT 'A serialized array of function names (like node_load) to be called to load an object corresponding to a part of the current path.',
  `to_arg_functions` blob NOT NULL COMMENT 'A serialized array of function names (like user_uid_optional_to_arg) to be called to replace a part of the router path with another string.',
  `access_callback` varchar(255) NOT NULL DEFAULT '' COMMENT 'The callback which determines the access to this router path. Defaults to user_access.',
  `access_arguments` blob DEFAULT NULL COMMENT 'A serialized array of arguments for the access callback.',
  `page_callback` varchar(255) NOT NULL DEFAULT '' COMMENT 'The name of the function that renders the page.',
  `page_arguments` blob DEFAULT NULL COMMENT 'A serialized array of arguments for the page callback.',
  `delivery_callback` varchar(255) NOT NULL DEFAULT '' COMMENT 'The name of the function that sends the result of the page_callback function to the browser.',
  `fit` int(11) NOT NULL DEFAULT 0 COMMENT 'A numeric representation of how specific the path is.',
  `number_parts` smallint(6) NOT NULL DEFAULT 0 COMMENT 'Number of parts in this router path.',
  `context` int(11) NOT NULL DEFAULT 0 COMMENT 'Only for local tasks (tabs) - the context of a local task to control its placement.',
  `tab_parent` varchar(255) NOT NULL DEFAULT '' COMMENT 'Only for local tasks (tabs) - the router path of the parent page (which may also be a local task).',
  `tab_root` varchar(255) NOT NULL DEFAULT '' COMMENT 'Router path of the closest non-tab parent page. For pages that are not local tasks, this will be the same as the path.',
  `title` varchar(255) NOT NULL DEFAULT '' COMMENT 'The title for the current page, or the title for the tab if this is a local task.',
  `title_callback` varchar(255) NOT NULL DEFAULT '' COMMENT 'A function which will alter the title. Defaults to t()',
  `title_arguments` varchar(255) NOT NULL DEFAULT '' COMMENT 'A serialized array of arguments for the title callback. If empty, the title will be used as the sole argument for the title callback.',
  `theme_callback` varchar(255) NOT NULL DEFAULT '' COMMENT 'A function which returns the name of the theme that will be used to render this page. If left empty, the default theme will be used.',
  `theme_arguments` varchar(255) NOT NULL DEFAULT '' COMMENT 'A serialized array of arguments for the theme callback.',
  `type` int(11) NOT NULL DEFAULT 0 COMMENT 'Numeric representation of the type of the menu item, like MENU_LOCAL_TASK.',
  `description` mediumtext NOT NULL COMMENT 'A description of this item.',
  `position` varchar(255) NOT NULL DEFAULT '' COMMENT 'The position of the block (left or right) on the system administration page for this item.',
  `weight` int(11) NOT NULL DEFAULT 0 COMMENT 'Weight of the element. Lighter weights are higher up, heavier weights go down.',
  `include_file` longtext DEFAULT NULL COMMENT 'The file to include for this element, usually the page callback function lives in this file.'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Maps paths to various callbacks (access, page and title)' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `messages`
--

CREATE TABLE `messages` (
  `id` int(11) NOT NULL,
  `postid` bigint(20) DEFAULT NULL COMMENT 'related post ID',
  `message` varchar(255) DEFAULT NULL COMMENT 'the message',
  `sender` bigint(20) DEFAULT NULL COMMENT 'pid of sender',
  `confirmed` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'confirmed by email',
  `created` int(11) DEFAULT NULL COMMENT 'creation date'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='messages responding to offers and needs' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `people`
--

CREATE TABLE `people` (
  `pid` bigint(20) NOT NULL COMMENT 'record ID',
  `uid` bigint(20) DEFAULT NULL COMMENT 'poster''s associated account ID, if any',
  `displayName` varchar(255) DEFAULT NULL COMMENT 'first name or nickname',
  `fullName` varchar(255) DEFAULT NULL COMMENT 'full name',
  `street` varchar(255) DEFAULT NULL COMMENT 'street address without street number or apt number',
  `address` varchar(255) DEFAULT NULL COMMENT 'physical address',
  `city` varchar(255) DEFAULT NULL COMMENT 'city',
  `state` mediumint(9) DEFAULT NULL COMMENT 'state',
  `zip` varchar(255) DEFAULT NULL COMMENT 'postal code',
  `country` mediumint(8) NOT NULL DEFAULT 1228 COMMENT 'country index',
  `phone` varchar(255) DEFAULT NULL COMMENT 'phone number',
  `email` varchar(255) DEFAULT NULL COMMENT 'email address',
  `method` enum('email','phone','text') DEFAULT NULL COMMENT 'preferred contact method',
  `latitude` decimal(11,8) NOT NULL DEFAULT 0.00000000 COMMENT 'latitude of location',
  `longitude` decimal(11,8) NOT NULL DEFAULT 0.00000000 COMMENT 'longitude of location',
  `notes` longtext DEFAULT NULL COMMENT 'miscellaneous notes about the person',
  `confirmed` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'confirmed by email',
  `created` int(11) DEFAULT NULL COMMENT 'creation date',
  `notices` text DEFAULT NULL COMMENT 'notice preferences',
  `health` varchar(255) NOT NULL DEFAULT '0' COMMENT 'summary of COVID19 survey answers',
  `source` mediumtext DEFAULT NULL COMMENT 'how did this person hear about us?'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='contact information for non-members' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `phinxlog`
--

CREATE TABLE `phinxlog` (
  `version` bigint(20) NOT NULL,
  `migration_name` varchar(100) DEFAULT NULL,
  `start_time` timestamp NULL DEFAULT NULL,
  `end_time` timestamp NULL DEFAULT NULL,
  `breakpoint` tinyint(1) NOT NULL DEFAULT 0
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC;

--
-- Dumping data for table `phinxlog`
--

INSERT INTO `phinxlog` VALUES(20190213232929, 'NewAgreementAndBacking', '2021-04-22 00:11:32', '2021-04-22 00:11:32', 0);
INSERT INTO `phinxlog` VALUES(20190311170655, 'PayWithCgLink', '2021-04-22 00:11:32', '2021-04-22 00:11:32', 0);
INSERT INTO `phinxlog` VALUES(20190322132020, 'FixSetlocus', '2021-04-22 00:11:32', '2021-04-22 00:11:32', 0);
INSERT INTO `phinxlog` VALUES(20190322132821, 'OneMetric', '2021-04-22 00:11:32', '2021-04-22 00:11:32', 0);
INSERT INTO `phinxlog` VALUES(20190506153458, 'BalanceSheet', '2021-04-22 00:11:32', '2021-04-22 00:11:32', 0);
INSERT INTO `phinxlog` VALUES(20190507010000, 'CreateTableTxHdrsAll', '2021-04-22 00:11:32', '2021-04-22 00:11:32', 0);
INSERT INTO `phinxlog` VALUES(20190507020000, 'CreateTableTxEntriesAll', '2021-04-22 00:11:32', '2021-04-22 00:11:32', 0);
INSERT INTO `phinxlog` VALUES(20190507030000, 'AlterUsdTables', '2021-04-22 00:11:32', '2021-04-22 00:11:32', 0);
INSERT INTO `phinxlog` VALUES(20190507040000, 'CreateTableTxDisputesAll', '2021-04-22 00:11:32', '2021-04-22 00:11:32', 0);
INSERT INTO `phinxlog` VALUES(20190507050000, 'AddCanonicalAccounts', '2021-04-22 00:11:32', '2021-04-22 00:11:32', 0);
INSERT INTO `phinxlog` VALUES(20190507060000, 'ReformatTransactions', '2021-04-22 00:11:32', '2021-04-22 00:11:32', 0);
INSERT INTO `phinxlog` VALUES(20190507070000, 'CreateBalanceTriggers', '2021-04-22 00:11:32', '2021-04-22 00:11:32', 0);
INSERT INTO `phinxlog` VALUES(20190507080000, 'InitializeBalances', '2021-04-22 00:11:32', '2021-04-22 00:11:32', 0);
INSERT INTO `phinxlog` VALUES(20190507090000, 'RenameOldTxsTable', '2021-04-22 00:11:32', '2021-04-22 00:11:32', 0);
INSERT INTO `phinxlog` VALUES(20190531232058, 'PostSplitFixes', '2021-04-22 00:11:32', '2021-04-22 00:11:32', 0);
INSERT INTO `phinxlog` VALUES(20190605135132, 'FixRecurring', '2021-04-22 00:11:32', '2021-04-22 00:11:32', 0);
INSERT INTO `phinxlog` VALUES(20190614035252, 'SignupTracking', '2021-04-22 00:11:32', '2021-04-22 00:11:32', 0);
INSERT INTO `phinxlog` VALUES(20190618174054, 'FoodPercents', '2021-04-22 00:11:32', '2021-04-22 00:11:32', 0);
INSERT INTO `phinxlog` VALUES(20190622131859, 'SimplifyTxs', '2021-04-22 00:11:32', '2021-04-22 00:11:32', 0);
INSERT INTO `phinxlog` VALUES(20190710134016, 'Invest', '2021-04-22 00:11:32', '2021-04-22 00:11:33', 0);
INSERT INTO `phinxlog` VALUES(20190727234655, 'RestrictedCoupons', '2021-04-22 00:11:33', '2021-04-22 00:11:33', 0);
INSERT INTO `phinxlog` VALUES(20190802200257, 'PiecemealDiscounts', '2021-04-22 00:11:33', '2021-04-22 00:11:33', 0);
INSERT INTO `phinxlog` VALUES(20190804180734, 'Whence', '2021-04-22 00:11:33', '2021-04-22 00:11:33', 0);
INSERT INTO `phinxlog` VALUES(20190810142735, 'CouponFixes', '2021-04-22 00:11:33', '2021-04-22 00:11:33', 0);
INSERT INTO `phinxlog` VALUES(20190820133520, 'Ezbiz', '2021-04-22 00:11:33', '2021-04-22 00:11:33', 0);
INSERT INTO `phinxlog` VALUES(20191002173152, 'SimpleSignup', '2021-04-22 00:11:33', '2021-04-22 00:11:33', 0);
INSERT INTO `phinxlog` VALUES(20191021133320, 'EasyTweaks', '2021-04-22 00:11:33', '2021-04-22 00:11:33', 0);
INSERT INTO `phinxlog` VALUES(20200224161017, 'MakeIndustriesRecursive', '2021-04-22 00:11:33', '2021-04-22 00:11:33', 0);
INSERT INTO `phinxlog` VALUES(20200303120100, 'CreateTableUGroups', '2021-04-22 00:11:33', '2021-04-22 00:11:33', 0);
INSERT INTO `phinxlog` VALUES(20200303120200, 'CreateTableUGroupies', '2021-04-22 00:11:33', '2021-04-22 00:11:33', 0);
INSERT INTO `phinxlog` VALUES(20200303120300, 'CreateTableTxTemplates', '2021-04-22 00:11:33', '2021-04-22 00:11:33', 0);
INSERT INTO `phinxlog` VALUES(20200303120400, 'CreateTableTxRules', '2021-04-22 00:11:33', '2021-04-22 00:11:33', 0);
INSERT INTO `phinxlog` VALUES(20200303120500, 'AddFieldsToTxEntriesAll', '2021-04-22 00:11:33', '2021-04-22 00:11:33', 0);
INSERT INTO `phinxlog` VALUES(20200303120600, 'MigrateCoupons', '2021-04-22 00:11:33', '2021-04-22 00:11:33', 0);
INSERT INTO `phinxlog` VALUES(20200303120700, 'HideCouponRelatedTables', '2021-04-22 00:11:33', '2021-04-22 00:11:33', 0);
INSERT INTO `phinxlog` VALUES(20200313220601, 'FreeOffers', '2021-04-22 00:11:33', '2021-04-22 00:11:33', 0);
INSERT INTO `phinxlog` VALUES(20200318025417, 'PostTweaks', '2021-04-22 00:11:33', '2021-04-22 00:11:33', 0);
INSERT INTO `phinxlog` VALUES(20200318181411, 'AddPostCat', '2021-04-22 00:11:33', '2021-04-22 00:11:33', 0);
INSERT INTO `phinxlog` VALUES(20200319170844, 'PostGeosearch', '2021-04-22 00:11:33', '2021-04-22 00:11:33', 0);
INSERT INTO `phinxlog` VALUES(20200325023154, 'PostFormat', '2021-04-22 00:11:33', '2021-04-22 00:11:33', 0);
INSERT INTO `phinxlog` VALUES(20200327225059, 'PostModerate', '2021-04-22 00:11:33', '2021-04-22 00:11:33', 0);
INSERT INTO `phinxlog` VALUES(20200328192849, 'PostTouchups2', '2021-04-22 00:11:33', '2021-04-22 00:11:33', 0);
INSERT INTO `phinxlog` VALUES(20200330015207, 'PostBugs', '2021-04-22 00:11:33', '2021-04-22 00:11:34', 0);
INSERT INTO `phinxlog` VALUES(20200405033914, 'PostTips', '2021-04-22 00:11:34', '2021-04-22 00:11:34', 0);
INSERT INTO `phinxlog` VALUES(20200428134609, 'RecursCentral', '2021-04-22 00:11:34', '2021-04-22 00:11:34', 0);
INSERT INTO `phinxlog` VALUES(20200428174926, 'PerYearFunction', '2021-04-22 00:11:34', '2021-04-22 00:11:34', 0);
INSERT INTO `phinxlog` VALUES(20200507233810, 'PostNotices', '2021-04-22 00:11:34', '2021-04-22 00:11:34', 0);
INSERT INTO `phinxlog` VALUES(20200511134354, 'Please', '2021-04-22 00:11:34', '2021-04-22 00:11:34', 0);
INSERT INTO `phinxlog` VALUES(20200526000522, 'PhotoPermission', '2021-04-22 00:11:34', '2021-04-22 00:11:34', 0);
INSERT INTO `phinxlog` VALUES(20200531124652, 'InviteAll', '2021-04-22 00:11:34', '2021-04-22 00:11:34', 0);
INSERT INTO `phinxlog` VALUES(20200608161335, 'InviteOverhaul', '2021-04-22 00:11:34', '2021-04-22 00:11:34', 0);
INSERT INTO `phinxlog` VALUES(20200702225507, 'PushEndorse', '2021-04-22 00:11:34', '2021-04-22 00:11:34', 0);
INSERT INTO `phinxlog` VALUES(20200707130318, 'RenewBacking', '2021-04-22 00:11:34', '2021-04-22 00:11:34', 0);
INSERT INTO `phinxlog` VALUES(20200903125352, 'Notices', '2021-04-22 00:11:34', '2021-04-22 00:11:34', 0);
INSERT INTO `phinxlog` VALUES(20200917034029, 'PostExpand', '2021-04-22 00:11:34', '2021-04-22 00:11:34', 0);
INSERT INTO `phinxlog` VALUES(20200929073504, 'Sweep', '2021-04-22 00:11:34', '2021-04-22 00:11:34', 0);
INSERT INTO `phinxlog` VALUES(20201012050311, 'Mb4', '2021-04-22 00:11:34', '2021-04-22 00:11:37', 0);
INSERT INTO `phinxlog` VALUES(20201018043604, 'NoBank', '2021-04-22 00:11:37', '2021-04-22 00:11:37', 0);
INSERT INTO `phinxlog` VALUES(20201028121138, 'ShortSignup', '2021-04-22 00:11:37', '2021-04-22 00:11:37', 0);
INSERT INTO `phinxlog` VALUES(20201124082459, 'Cttys', '2021-04-22 00:11:37', '2021-04-22 00:11:37', 0);
INSERT INTO `phinxlog` VALUES(20201201053609, 'PgpDeviceId', '2021-04-22 00:11:37', '2021-04-22 00:11:37', 0);
INSERT INTO `phinxlog` VALUES(20210518071537, 'Unpay', '2024-02-02 20:58:03', '2024-02-02 20:58:03', 0);
INSERT INTO `phinxlog` VALUES(20210527100150, 'Sponsor', '2024-02-02 20:58:03', '2024-02-02 20:58:03', 0);
INSERT INTO `phinxlog` VALUES(20210607111225, 'Invoices', '2024-02-02 20:58:03', '2024-02-02 20:58:03', 0);
INSERT INTO `phinxlog` VALUES(20210623090725, 'Logo', '2024-02-02 20:58:03', '2024-02-02 20:58:03', 0);
INSERT INTO `phinxlog` VALUES(20210702120044, 'Txs2', '2024-02-02 20:58:03', '2024-02-02 20:58:04', 0);
INSERT INTO `phinxlog` VALUES(20210706122107, 'Fbo2', '2024-02-02 20:58:04', '2024-02-02 20:58:04', 0);
INSERT INTO `phinxlog` VALUES(20210706123242, 'MoveCat', '2024-02-02 20:58:04', '2024-02-02 20:58:05', 0);
INSERT INTO `phinxlog` VALUES(20210706144217, 'MakeOuter', '2024-02-02 20:58:05', '2024-02-02 20:58:05', 0);
INSERT INTO `phinxlog` VALUES(20210710025233, 'Fbo3', '2024-02-02 20:58:05', '2024-02-02 20:58:05', 0);
INSERT INTO `phinxlog` VALUES(20210713110102, 'Zips', '2024-02-02 20:58:05', '2024-02-02 20:58:10', 0);
INSERT INTO `phinxlog` VALUES(20210725025306, 'Founded', '2024-02-02 20:58:10', '2024-02-02 20:58:10', 0);
INSERT INTO `phinxlog` VALUES(20210912053426, 'Deletion', '2024-02-02 20:58:10', '2024-02-02 20:58:10', 0);
INSERT INTO `phinxlog` VALUES(20211108030520, 'SimplerSignup', '2024-02-02 20:58:10', '2024-02-02 20:58:10', 0);
INSERT INTO `phinxlog` VALUES(20211209042914, 'Supers', '2024-02-02 20:58:10', '2024-02-02 20:58:10', 0);
INSERT INTO `phinxlog` VALUES(20211230110407, 'Admins', '2024-02-02 20:58:10', '2024-02-02 20:58:10', 0);
INSERT INTO `phinxlog` VALUES(20220211045249, 'CcGifts', '2024-02-02 20:58:10', '2024-02-02 20:58:10', 0);
INSERT INTO `phinxlog` VALUES(20220222023529, 'MinimalAdmin', '2024-02-02 20:58:10', '2024-02-02 20:58:11', 0);
INSERT INTO `phinxlog` VALUES(20220318124031, 'QbAccounts', '2024-02-02 20:58:11', '2024-02-02 20:58:11', 0);
INSERT INTO `phinxlog` VALUES(20220408092039, 'Cg2qb', '2024-02-02 20:58:11', '2024-02-02 20:58:11', 0);
INSERT INTO `phinxlog` VALUES(20220412044909, 'SetCats', '2024-02-02 20:58:11', '2024-02-02 20:58:11', 0);
INSERT INTO `phinxlog` VALUES(20220504104223, 'QbTxs', '2024-02-02 20:58:11', '2024-02-02 20:58:11', 0);
INSERT INTO `phinxlog` VALUES(20220512080235, 'QbCttyFund', '2024-02-02 20:58:11', '2024-02-02 20:58:11', 0);
INSERT INTO `phinxlog` VALUES(20220521071358, 'Qbo3', '2024-02-02 20:58:11', '2024-02-02 20:58:12', 0);
INSERT INTO `phinxlog` VALUES(20220529025942, 'CatTest', '2024-02-02 20:58:12', '2024-02-02 20:58:12', 0);
INSERT INTO `phinxlog` VALUES(20220606070942, 'Clickup2', '2024-02-02 20:58:12', '2024-02-02 20:58:12', 0);
INSERT INTO `phinxlog` VALUES(20220613051718, 'Sql', '2024-02-02 20:58:12', '2024-02-02 20:58:12', 0);
INSERT INTO `phinxlog` VALUES(20220717035808, 'Loyalty', '2024-02-02 20:58:12', '2024-02-02 20:58:13', 0);
INSERT INTO `phinxlog` VALUES(20221021104444, 'DescLen', '2024-02-02 20:58:13', '2024-02-02 20:58:13', 0);
INSERT INTO `phinxlog` VALUES(20230224101130, 'Comments', '2024-02-02 20:58:13', '2024-02-02 20:58:13', 0);
INSERT INTO `phinxlog` VALUES(20230527033654, 'BadsMsg', '2024-02-02 20:58:13', '2024-02-02 20:58:13', 0);
INSERT INTO `phinxlog` VALUES(20230527051322, 'BadNegAmt', '2024-02-02 20:58:13', '2024-02-02 20:58:13', 0);
INSERT INTO `phinxlog` VALUES(20230811023357, 'Thermometer', '2024-02-02 20:58:13', '2024-02-02 20:58:13', 0);
INSERT INTO `phinxlog` VALUES(20230816065722, 'CreateSponsee', '2024-02-02 20:58:13', '2024-02-02 20:58:13', 0);
INSERT INTO `phinxlog` VALUES(20230930015403, 'Cap', '2024-02-02 20:58:13', '2024-02-02 20:58:13', 0);
INSERT INTO `phinxlog` VALUES(20231018071151, 'BigBadField', '2024-02-02 20:58:13', '2024-02-02 20:58:13', 0);

-- --------------------------------------------------------

--
-- Table structure for table `posts`
--

CREATE TABLE `posts` (
  `postid` bigint(20) NOT NULL COMMENT 'record ID',
  `type` enum('need','offer','tip') DEFAULT NULL COMMENT 'item type',
  `item` varchar(255) DEFAULT NULL COMMENT 'item offered or needed',
  `details` longtext DEFAULT NULL COMMENT 'description of item',
  `cat` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'service category',
  `service` tinyint(4) DEFAULT NULL COMMENT '0=goods 1=service',
  `exchange` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'is exchange wanted?',
  `emergency` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'because of a short-term emergency',
  `radius` decimal(11,6) DEFAULT NULL COMMENT 'geographic limit of post visibility, in miles',
  `pid` bigint(20) DEFAULT NULL COMMENT 'poster''s associated people record ID, if any',
  `hits` bigint(20) NOT NULL DEFAULT 0 COMMENT 'number of times details about this item have been viewed',
  `contacts` bigint(20) NOT NULL DEFAULT 0 COMMENT 'number of contacts about this item',
  `confirmed` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'confirmed by email',
  `created` int(11) DEFAULT NULL COMMENT 'start date',
  `end` int(11) DEFAULT NULL COMMENT 'end date',
  `private` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'show this post only to the administrator'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='offers and needs posted by members and non-members' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `post_cats`
--

CREATE TABLE `post_cats` (
  `id` int(11) NOT NULL,
  `cat` varchar(255) DEFAULT NULL COMMENT 'category of service',
  `sort` mediumint(9) NOT NULL DEFAULT 0 COMMENT 'sorting order'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='types of offers and needs' ROW_FORMAT=DYNAMIC;

--
-- Dumping data for table `post_cats`
--

INSERT INTO `post_cats` VALUES(1, 'food', 100);
INSERT INTO `post_cats` VALUES(2, 'housing', 200);
INSERT INTO `post_cats` VALUES(4, 'health', 400);
INSERT INTO `post_cats` VALUES(5, 'travel/rides', 1930);
INSERT INTO `post_cats` VALUES(6, 'delivery', 600);
INSERT INTO `post_cats` VALUES(7, 'childcare', 700);
INSERT INTO `post_cats` VALUES(9, 'animal care', 900);
INSERT INTO `post_cats` VALUES(10, 'cleaning', 1910);
INSERT INTO `post_cats` VALUES(11, 'legal', 1100);
INSERT INTO `post_cats` VALUES(13, 'fellowship', 1300);
INSERT INTO `post_cats` VALUES(14, 'muscle/labor', 1400);
INSERT INTO `post_cats` VALUES(16, 'technology', 1600);
INSERT INTO `post_cats` VALUES(17, 'info/training', 1700);
INSERT INTO `post_cats` VALUES(19, 'finance/money', 1900);
INSERT INTO `post_cats` VALUES(20, 'other', 2000);

-- --------------------------------------------------------

--
-- Table structure for table `queue`
--

CREATE TABLE `queue` (
  `id` bigint(20) NOT NULL COMMENT 'primary key: Unique item ID',
  `item` longblob DEFAULT NULL COMMENT 'arbitrary data for the item.',
  `created` int(11) DEFAULT NULL COMMENT 'Unixtime record was created'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Cron queue' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `registry`
--

CREATE TABLE `registry` (
  `name` varchar(255) NOT NULL DEFAULT '' COMMENT 'The name of the function, class, or interface.',
  `type` varchar(9) NOT NULL DEFAULT '' COMMENT 'Either function or class or interface.',
  `filename` varchar(255) NOT NULL COMMENT 'Name of the file.',
  `module` varchar(255) NOT NULL DEFAULT '' COMMENT 'Name of the module the file belongs to.',
  `weight` int(11) NOT NULL DEFAULT 0 COMMENT 'The order in which this module’s hooks should be invoked relative to other modules. Equal-weighted modules are ordered by name.'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Each record is a function, class, or interface name and...' ROW_FORMAT=DYNAMIC;

--
-- Dumping data for table `registry`
--

INSERT INTO `registry` VALUES('AccessDeniedTestCase', 'class', 'modules/system/system.test', 'system', 0);
INSERT INTO `registry` VALUES('AdminMetaTagTestCase', 'class', 'modules/system/system.test', 'system', 0);
INSERT INTO `registry` VALUES('Archive_Tar', 'class', 'modules/system/system.tar.inc', 'system', 0);
INSERT INTO `registry` VALUES('ArchiverInterface', 'interface', 'includes/archiver.inc', '', 0);
INSERT INTO `registry` VALUES('ArchiverTar', 'class', 'modules/system/system.archiver.inc', 'system', 0);
INSERT INTO `registry` VALUES('ArchiverZip', 'class', 'modules/system/system.archiver.inc', 'system', 0);
INSERT INTO `registry` VALUES('BatchMemoryQueue', 'class', 'includes/batch.queue.inc', '', 0);
INSERT INTO `registry` VALUES('BatchQueue', 'class', 'includes/batch.queue.inc', '', 0);
INSERT INTO `registry` VALUES('BlockAdminThemeTestCase', 'class', 'modules/block/block.test', 'block', -5);
INSERT INTO `registry` VALUES('BlockCacheTestCase', 'class', 'modules/block/block.test', 'block', -5);
INSERT INTO `registry` VALUES('BlockHashTestCase', 'class', 'modules/block/block.test', 'block', -5);
INSERT INTO `registry` VALUES('BlockHiddenRegionTestCase', 'class', 'modules/block/block.test', 'block', -5);
INSERT INTO `registry` VALUES('BlockHTMLIdTestCase', 'class', 'modules/block/block.test', 'block', -5);
INSERT INTO `registry` VALUES('BlockInvalidRegionTestCase', 'class', 'modules/block/block.test', 'block', -5);
INSERT INTO `registry` VALUES('BlockTemplateSuggestionsUnitTest', 'class', 'modules/block/block.test', 'block', -5);
INSERT INTO `registry` VALUES('BlockTestCase', 'class', 'modules/block/block.test', 'block', -5);
INSERT INTO `registry` VALUES('BlockViewModuleDeltaAlterWebTest', 'class', 'modules/block/block.test', 'block', -5);
INSERT INTO `registry` VALUES('call', 'class', 'rcredits/rsms/rsms-call.inc', 'rsms', 0);
INSERT INTO `registry` VALUES('CronQueueTestCase', 'class', 'modules/system/system.test', 'system', 0);
INSERT INTO `registry` VALUES('CronRunTestCase', 'class', 'modules/system/system.test', 'system', 0);
INSERT INTO `registry` VALUES('Database', 'class', 'includes/database/database.inc', '', 0);
INSERT INTO `registry` VALUES('DatabaseCondition', 'class', 'includes/database/query.inc', '', 0);
INSERT INTO `registry` VALUES('DatabaseConnection', 'class', 'includes/database/database.inc', '', 0);
INSERT INTO `registry` VALUES('DatabaseConnection_mysql', 'class', 'includes/database/mysql/database.inc', '', 0);
INSERT INTO `registry` VALUES('DatabaseConnection_pgsql', 'class', 'includes/database/pgsql/database.inc', '', 0);
INSERT INTO `registry` VALUES('DatabaseConnection_sqlite', 'class', 'includes/database/sqlite/database.inc', '', 0);
INSERT INTO `registry` VALUES('DatabaseConnectionNotDefinedException', 'class', 'includes/database/database.inc', '', 0);
INSERT INTO `registry` VALUES('DatabaseDriverNotSpecifiedException', 'class', 'includes/database/database.inc', '', 0);
INSERT INTO `registry` VALUES('DatabaseLog', 'class', 'includes/database/log.inc', '', 0);
INSERT INTO `registry` VALUES('DatabaseSchema', 'class', 'includes/database/schema.inc', '', 0);
INSERT INTO `registry` VALUES('DatabaseSchema_mysql', 'class', 'includes/database/mysql/schema.inc', '', 0);
INSERT INTO `registry` VALUES('DatabaseSchema_pgsql', 'class', 'includes/database/pgsql/schema.inc', '', 0);
INSERT INTO `registry` VALUES('DatabaseSchema_sqlite', 'class', 'includes/database/sqlite/schema.inc', '', 0);
INSERT INTO `registry` VALUES('DatabaseSchemaObjectDoesNotExistException', 'class', 'includes/database/schema.inc', '', 0);
INSERT INTO `registry` VALUES('DatabaseSchemaObjectExistsException', 'class', 'includes/database/schema.inc', '', 0);
INSERT INTO `registry` VALUES('DatabaseStatement_sqlite', 'class', 'includes/database/sqlite/database.inc', '', 0);
INSERT INTO `registry` VALUES('DatabaseStatementBase', 'class', 'includes/database/database.inc', '', 0);
INSERT INTO `registry` VALUES('DatabaseStatementEmpty', 'class', 'includes/database/database.inc', '', 0);
INSERT INTO `registry` VALUES('DatabaseStatementInterface', 'interface', 'includes/database/database.inc', '', 0);
INSERT INTO `registry` VALUES('DatabaseStatementPrefetch', 'class', 'includes/database/prefetch.inc', '', 0);
INSERT INTO `registry` VALUES('DatabaseTaskException', 'class', 'includes/install.inc', '', 0);
INSERT INTO `registry` VALUES('DatabaseTasks', 'class', 'includes/install.inc', '', 0);
INSERT INTO `registry` VALUES('DatabaseTasks_mysql', 'class', 'includes/database/mysql/install.inc', '', 0);
INSERT INTO `registry` VALUES('DatabaseTasks_pgsql', 'class', 'includes/database/pgsql/install.inc', '', 0);
INSERT INTO `registry` VALUES('DatabaseTasks_sqlite', 'class', 'includes/database/sqlite/install.inc', '', 0);
INSERT INTO `registry` VALUES('DatabaseTransaction', 'class', 'includes/database/database.inc', '', 0);
INSERT INTO `registry` VALUES('DatabaseTransactionCommitFailedException', 'class', 'includes/database/database.inc', '', 0);
INSERT INTO `registry` VALUES('DatabaseTransactionExplicitCommitNotAllowedException', 'class', 'includes/database/database.inc', '', 0);
INSERT INTO `registry` VALUES('DatabaseTransactionNameNonUniqueException', 'class', 'includes/database/database.inc', '', 0);
INSERT INTO `registry` VALUES('DatabaseTransactionNoActiveException', 'class', 'includes/database/database.inc', '', 0);
INSERT INTO `registry` VALUES('DatabaseTransactionOutOfOrderException', 'class', 'includes/database/database.inc', '', 0);
INSERT INTO `registry` VALUES('DateTimeFunctionalTest', 'class', 'modules/system/system.test', 'system', 0);
INSERT INTO `registry` VALUES('DefaultMailSystem', 'class', 'modules/system/system.mail.inc', 'system', 0);
INSERT INTO `registry` VALUES('DeleteQuery', 'class', 'includes/database/query.inc', '', 0);
INSERT INTO `registry` VALUES('DeleteQuery_sqlite', 'class', 'includes/database/sqlite/query.inc', '', 0);
INSERT INTO `registry` VALUES('DevelMailLog', 'class', 'sites/all/modules/devel/devel.mail.inc', 'devel', 0);
INSERT INTO `registry` VALUES('DevelMailTest', 'class', 'sites/all/modules/devel/devel.test', 'devel', 0);
INSERT INTO `registry` VALUES('DrupalCacheArray', 'class', 'includes/bootstrap.inc', '', 0);
INSERT INTO `registry` VALUES('DrupalCacheInterface', 'interface', 'includes/cache.inc', '', 0);
INSERT INTO `registry` VALUES('DrupalDatabaseCache', 'class', 'includes/cache.inc', '', 0);
INSERT INTO `registry` VALUES('DrupalDefaultEntityController', 'class', 'includes/entity.inc', '', 0);
INSERT INTO `registry` VALUES('DrupalEntityControllerInterface', 'interface', 'includes/entity.inc', '', 0);
INSERT INTO `registry` VALUES('DrupalFakeCache', 'class', 'includes/cache-install.inc', '', 0);
INSERT INTO `registry` VALUES('DrupalLocalStreamWrapper', 'class', 'includes/stream_wrappers.inc', '', 0);
INSERT INTO `registry` VALUES('DrupalPrivateStreamWrapper', 'class', 'includes/stream_wrappers.inc', '', 0);
INSERT INTO `registry` VALUES('DrupalPublicStreamWrapper', 'class', 'includes/stream_wrappers.inc', '', 0);
INSERT INTO `registry` VALUES('DrupalQueue', 'class', 'modules/system/system.queue.inc', 'system', 0);
INSERT INTO `registry` VALUES('DrupalQueueInterface', 'interface', 'modules/system/system.queue.inc', 'system', 0);
INSERT INTO `registry` VALUES('DrupalReliableQueueInterface', 'interface', 'modules/system/system.queue.inc', 'system', 0);
INSERT INTO `registry` VALUES('DrupalStreamWrapperInterface', 'interface', 'includes/stream_wrappers.inc', '', 0);
INSERT INTO `registry` VALUES('DrupalTemporaryStreamWrapper', 'class', 'includes/stream_wrappers.inc', '', 0);
INSERT INTO `registry` VALUES('DrupalUpdateException', 'class', 'includes/update.inc', '', 0);
INSERT INTO `registry` VALUES('DrupalUpdaterInterface', 'interface', 'includes/updater.inc', '', 0);
INSERT INTO `registry` VALUES('EnableDisableTestCase', 'class', 'modules/system/system.test', 'system', 0);
INSERT INTO `registry` VALUES('EntityFieldQuery', 'class', 'includes/entity.inc', '', 0);
INSERT INTO `registry` VALUES('EntityFieldQueryException', 'class', 'includes/entity.inc', '', 0);
INSERT INTO `registry` VALUES('EntityMalformedException', 'class', 'includes/entity.inc', '', 0);
INSERT INTO `registry` VALUES('EntityPropertiesTestCase', 'class', 'modules/field/tests/field.test', 'field', 0);
INSERT INTO `registry` VALUES('FieldAttachOtherTestCase', 'class', 'modules/field/tests/field.test', 'field', 0);
INSERT INTO `registry` VALUES('FieldAttachStorageTestCase', 'class', 'modules/field/tests/field.test', 'field', 0);
INSERT INTO `registry` VALUES('FieldAttachTestCase', 'class', 'modules/field/tests/field.test', 'field', 0);
INSERT INTO `registry` VALUES('FieldBulkDeleteTestCase', 'class', 'modules/field/tests/field.test', 'field', 0);
INSERT INTO `registry` VALUES('FieldCrudTestCase', 'class', 'modules/field/tests/field.test', 'field', 0);
INSERT INTO `registry` VALUES('FieldDisplayAPITestCase', 'class', 'modules/field/tests/field.test', 'field', 0);
INSERT INTO `registry` VALUES('FieldException', 'class', 'modules/field/field.module', 'field', 0);
INSERT INTO `registry` VALUES('FieldFormTestCase', 'class', 'modules/field/tests/field.test', 'field', 0);
INSERT INTO `registry` VALUES('FieldInfo', 'class', 'modules/field/field.info.class.inc', 'field', 0);
INSERT INTO `registry` VALUES('FieldInfoTestCase', 'class', 'modules/field/tests/field.test', 'field', 0);
INSERT INTO `registry` VALUES('FieldInstanceCrudTestCase', 'class', 'modules/field/tests/field.test', 'field', 0);
INSERT INTO `registry` VALUES('FieldsOverlapException', 'class', 'includes/database/database.inc', '', 0);
INSERT INTO `registry` VALUES('FieldSqlStorageTestCase', 'class', 'modules/field/modules/field_sql_storage/field_sql_storage.test', 'field_sql_storage', 0);
INSERT INTO `registry` VALUES('FieldTestCase', 'class', 'modules/field/tests/field.test', 'field', 0);
INSERT INTO `registry` VALUES('FieldTranslationsTestCase', 'class', 'modules/field/tests/field.test', 'field', 0);
INSERT INTO `registry` VALUES('FieldUpdateForbiddenException', 'class', 'modules/field/field.module', 'field', 0);
INSERT INTO `registry` VALUES('FieldValidationException', 'class', 'modules/field/field.attach.inc', 'field', 0);
INSERT INTO `registry` VALUES('FileFieldDisplayTestCase', 'class', 'modules/file/tests/file.test', 'file', 0);
INSERT INTO `registry` VALUES('FileFieldPathTestCase', 'class', 'modules/file/tests/file.test', 'file', 0);
INSERT INTO `registry` VALUES('FileFieldRevisionTestCase', 'class', 'modules/file/tests/file.test', 'file', 0);
INSERT INTO `registry` VALUES('FileFieldTestCase', 'class', 'modules/file/tests/file.test', 'file', 0);
INSERT INTO `registry` VALUES('FileFieldValidateTestCase', 'class', 'modules/file/tests/file.test', 'file', 0);
INSERT INTO `registry` VALUES('FileFieldWidgetTestCase', 'class', 'modules/file/tests/file.test', 'file', 0);
INSERT INTO `registry` VALUES('FileManagedFileElementTestCase', 'class', 'modules/file/tests/file.test', 'file', 0);
INSERT INTO `registry` VALUES('FilePrivateTestCase', 'class', 'modules/file/tests/file.test', 'file', 0);
INSERT INTO `registry` VALUES('FileTaxonomyTermTestCase', 'class', 'modules/file/tests/file.test', 'file', 0);
INSERT INTO `registry` VALUES('FileTokenReplaceTestCase', 'class', 'modules/file/tests/file.test', 'file', 0);
INSERT INTO `registry` VALUES('FileTransfer', 'class', 'includes/filetransfer/filetransfer.inc', '', 0);
INSERT INTO `registry` VALUES('FileTransferChmodInterface', 'interface', 'includes/filetransfer/filetransfer.inc', '', 0);
INSERT INTO `registry` VALUES('FileTransferException', 'class', 'includes/filetransfer/filetransfer.inc', '', 0);
INSERT INTO `registry` VALUES('FileTransferFTP', 'class', 'includes/filetransfer/ftp.inc', '', 0);
INSERT INTO `registry` VALUES('FileTransferFTPExtension', 'class', 'includes/filetransfer/ftp.inc', '', 0);
INSERT INTO `registry` VALUES('FileTransferLocal', 'class', 'includes/filetransfer/local.inc', '', 0);
INSERT INTO `registry` VALUES('FileTransferSSH', 'class', 'includes/filetransfer/ssh.inc', '', 0);
INSERT INTO `registry` VALUES('FilterAdminTestCase', 'class', 'modules/filter/filter.test', 'filter', 0);
INSERT INTO `registry` VALUES('FilterCRUDTestCase', 'class', 'modules/filter/filter.test', 'filter', 0);
INSERT INTO `registry` VALUES('FilterDefaultFormatTestCase', 'class', 'modules/filter/filter.test', 'filter', 0);
INSERT INTO `registry` VALUES('FilterFormatAccessTestCase', 'class', 'modules/filter/filter.test', 'filter', 0);
INSERT INTO `registry` VALUES('FilterHooksTestCase', 'class', 'modules/filter/filter.test', 'filter', 0);
INSERT INTO `registry` VALUES('FilterNoFormatTestCase', 'class', 'modules/filter/filter.test', 'filter', 0);
INSERT INTO `registry` VALUES('FilterSecurityTestCase', 'class', 'modules/filter/filter.test', 'filter', 0);
INSERT INTO `registry` VALUES('FilterSettingsTestCase', 'class', 'modules/filter/filter.test', 'filter', 0);
INSERT INTO `registry` VALUES('FilterUnitTestCase', 'class', 'modules/filter/filter.test', 'filter', 0);
INSERT INTO `registry` VALUES('FloodFunctionalTest', 'class', 'modules/system/system.test', 'system', 0);
INSERT INTO `registry` VALUES('FrontPageTestCase', 'class', 'modules/system/system.test', 'system', 0);
INSERT INTO `registry` VALUES('HookRequirementsTestCase', 'class', 'modules/system/system.test', 'system', 0);
INSERT INTO `registry` VALUES('InfoFileParserTestCase', 'class', 'modules/system/system.test', 'system', 0);
INSERT INTO `registry` VALUES('InsertQuery', 'class', 'includes/database/query.inc', '', 0);
INSERT INTO `registry` VALUES('InsertQuery_mysql', 'class', 'includes/database/mysql/query.inc', '', 0);
INSERT INTO `registry` VALUES('InsertQuery_pgsql', 'class', 'includes/database/pgsql/query.inc', '', 0);
INSERT INTO `registry` VALUES('InsertQuery_sqlite', 'class', 'includes/database/sqlite/query.inc', '', 0);
INSERT INTO `registry` VALUES('InvalidMergeQueryException', 'class', 'includes/database/database.inc', '', 0);
INSERT INTO `registry` VALUES('IPAddressBlockingTestCase', 'class', 'modules/system/system.test', 'system', 0);
INSERT INTO `registry` VALUES('MailSystemInterface', 'interface', 'includes/mail.inc', '', 0);
INSERT INTO `registry` VALUES('MemoryQueue', 'class', 'modules/system/system.queue.inc', 'system', 0);
INSERT INTO `registry` VALUES('MergeQuery', 'class', 'includes/database/query.inc', '', 0);
INSERT INTO `registry` VALUES('ModuleDependencyTestCase', 'class', 'modules/system/system.test', 'system', 0);
INSERT INTO `registry` VALUES('ModuleRequiredTestCase', 'class', 'modules/system/system.test', 'system', 0);
INSERT INTO `registry` VALUES('ModuleTestCase', 'class', 'modules/system/system.test', 'system', 0);
INSERT INTO `registry` VALUES('ModuleUpdater', 'class', 'modules/system/system.updater.inc', 'system', 0);
INSERT INTO `registry` VALUES('ModuleVersionTestCase', 'class', 'modules/system/system.test', 'system', 0);
INSERT INTO `registry` VALUES('MultiStepNodeFormBasicOptionsTest', 'class', 'modules/node/node.test', 'node', 0);
INSERT INTO `registry` VALUES('NewDefaultThemeBlocks', 'class', 'modules/block/block.test', 'block', -5);
INSERT INTO `registry` VALUES('NodeAccessBaseTableTestCase', 'class', 'modules/node/node.test', 'node', 0);
INSERT INTO `registry` VALUES('NodeAccessFieldTestCase', 'class', 'modules/node/node.test', 'node', 0);
INSERT INTO `registry` VALUES('NodeAccessPagerTestCase', 'class', 'modules/node/node.test', 'node', 0);
INSERT INTO `registry` VALUES('NodeAccessRebuildTestCase', 'class', 'modules/node/node.test', 'node', 0);
INSERT INTO `registry` VALUES('NodeAccessRecordsTestCase', 'class', 'modules/node/node.test', 'node', 0);
INSERT INTO `registry` VALUES('NodeAccessTestCase', 'class', 'modules/node/node.test', 'node', 0);
INSERT INTO `registry` VALUES('NodeAdminTestCase', 'class', 'modules/node/node.test', 'node', 0);
INSERT INTO `registry` VALUES('NodeBlockFunctionalTest', 'class', 'modules/node/node.test', 'node', 0);
INSERT INTO `registry` VALUES('NodeBlockTestCase', 'class', 'modules/node/node.test', 'node', 0);
INSERT INTO `registry` VALUES('NodeBuildContent', 'class', 'modules/node/node.test', 'node', 0);
INSERT INTO `registry` VALUES('NodeController', 'class', 'modules/node/node.module', 'node', 0);
INSERT INTO `registry` VALUES('NodeCreationTestCase', 'class', 'modules/node/node.test', 'node', 0);
INSERT INTO `registry` VALUES('NodeEntityFieldQueryAlter', 'class', 'modules/node/node.test', 'node', 0);
INSERT INTO `registry` VALUES('NodeEntityViewModeAlterTest', 'class', 'modules/node/node.test', 'node', 0);
INSERT INTO `registry` VALUES('NodeFeedTestCase', 'class', 'modules/node/node.test', 'node', 0);
INSERT INTO `registry` VALUES('NodeLoadHooksTestCase', 'class', 'modules/node/node.test', 'node', 0);
INSERT INTO `registry` VALUES('NodeLoadMultipleTestCase', 'class', 'modules/node/node.test', 'node', 0);
INSERT INTO `registry` VALUES('NodePageCacheTest', 'class', 'modules/node/node.test', 'node', 0);
INSERT INTO `registry` VALUES('NodePostSettingsTestCase', 'class', 'modules/node/node.test', 'node', 0);
INSERT INTO `registry` VALUES('NodeQueryAlter', 'class', 'modules/node/node.test', 'node', 0);
INSERT INTO `registry` VALUES('NodeRevisionPermissionsTestCase', 'class', 'modules/node/node.test', 'node', 0);
INSERT INTO `registry` VALUES('NodeRevisionsTestCase', 'class', 'modules/node/node.test', 'node', 0);
INSERT INTO `registry` VALUES('NodeRSSContentTestCase', 'class', 'modules/node/node.test', 'node', 0);
INSERT INTO `registry` VALUES('NodeSaveTestCase', 'class', 'modules/node/node.test', 'node', 0);
INSERT INTO `registry` VALUES('NodeTitleTestCase', 'class', 'modules/node/node.test', 'node', 0);
INSERT INTO `registry` VALUES('NodeTitleXSSTestCase', 'class', 'modules/node/node.test', 'node', 0);
INSERT INTO `registry` VALUES('NodeTokenReplaceTestCase', 'class', 'modules/node/node.test', 'node', 0);
INSERT INTO `registry` VALUES('NodeTypePersistenceTestCase', 'class', 'modules/node/node.test', 'node', 0);
INSERT INTO `registry` VALUES('NodeTypeTestCase', 'class', 'modules/node/node.test', 'node', 0);
INSERT INTO `registry` VALUES('NodeWebTestCase', 'class', 'modules/node/node.test', 'node', 0);
INSERT INTO `registry` VALUES('NoFieldsException', 'class', 'includes/database/database.inc', '', 0);
INSERT INTO `registry` VALUES('NonDefaultBlockAdmin', 'class', 'modules/block/block.test', 'block', -5);
INSERT INTO `registry` VALUES('PageEditTestCase', 'class', 'modules/node/node.test', 'node', 0);
INSERT INTO `registry` VALUES('PageNotFoundTestCase', 'class', 'modules/system/system.test', 'system', 0);
INSERT INTO `registry` VALUES('PagePreviewTestCase', 'class', 'modules/node/node.test', 'node', 0);
INSERT INTO `registry` VALUES('PagerDefault', 'class', 'includes/pager.inc', '', 0);
INSERT INTO `registry` VALUES('PageTitleFiltering', 'class', 'modules/system/system.test', 'system', 0);
INSERT INTO `registry` VALUES('PageViewTestCase', 'class', 'modules/node/node.test', 'node', 0);
INSERT INTO `registry` VALUES('Query', 'class', 'includes/database/query.inc', '', 0);
INSERT INTO `registry` VALUES('QueryAlterableInterface', 'interface', 'includes/database/query.inc', '', 0);
INSERT INTO `registry` VALUES('QueryConditionInterface', 'interface', 'includes/database/query.inc', '', 0);
INSERT INTO `registry` VALUES('QueryExtendableInterface', 'interface', 'includes/database/select.inc', '', 0);
INSERT INTO `registry` VALUES('QueryPlaceholderInterface', 'interface', 'includes/database/query.inc', '', 0);
INSERT INTO `registry` VALUES('QueueTestCase', 'class', 'modules/system/system.test', 'system', 0);
INSERT INTO `registry` VALUES('RetrieveFileTestCase', 'class', 'modules/system/system.test', 'system', 0);
INSERT INTO `registry` VALUES('rsmartadhoc', 'class', 'rcredits/rsmart/tests/adhoc.test', 'rsmart', 0);
INSERT INTO `registry` VALUES('rsmartChange', 'class', 'rcredits/rsmart/tests/Change.test', 'rsmart', 0);
INSERT INTO `registry` VALUES('rsmartIdentifyQR', 'class', 'rcredits/rsmart/tests/IdentifyQR.test', 'rsmart', 0);
INSERT INTO `registry` VALUES('rsmartInsufficient', 'class', 'rcredits/rsmart/tests/Insufficient.test', 'rsmart', 0);
INSERT INTO `registry` VALUES('rsmartStartup', 'class', 'rcredits/rsmart/tests/Startup.test', 'rsmart', 0);
INSERT INTO `registry` VALUES('rsmartTransact', 'class', 'rcredits/rsmart/tests/Transact.test', 'rsmart', 0);
INSERT INTO `registry` VALUES('rsmartUndo', 'class', 'rcredits/rsmart/tests/Undo.test', 'rsmart', 0);
INSERT INTO `registry` VALUES('rsmartUndoAttack', 'class', 'rcredits/rsmart/tests/UndoAttack.test', 'rsmart', 0);
INSERT INTO `registry` VALUES('rsmartUndoCompleted', 'class', 'rcredits/rsmart/tests/UndoCompleted.test', 'rsmart', 0);
INSERT INTO `registry` VALUES('rsmartUndoPending', 'class', 'rcredits/rsmart/tests/UndoPending.test', 'rsmart', 0);
INSERT INTO `registry` VALUES('rsmartUnilateral', 'class', 'rcredits/rsmart/tests/Unilateral.test', 'rsmart', 0);
INSERT INTO `registry` VALUES('rsmsAbbreviationsWork', 'class', 'rcredits/rsms/tests/AbbreviationsWork.test', 'rsms', 0);
INSERT INTO `registry` VALUES('rSMSexception', 'class', 'rcredits/rsms/rsms-call.inc', 'rsms', 0);
INSERT INTO `registry` VALUES('rsmsExchangeForCash', 'class', 'rcredits/rsms/tests/ExchangeForCash.test', 'rsms', 0);
INSERT INTO `registry` VALUES('rsmsGetHelp', 'class', 'rcredits/rsms/tests/GetHelp.test', 'rsms', 0);
INSERT INTO `registry` VALUES('rsmsGetInformation', 'class', 'rcredits/rsms/tests/GetInformation.test', 'rsms', 0);
INSERT INTO `registry` VALUES('rsmsOfferToExchangeUSDollarsForRCredits', 'class', 'rcredits/rsms/tests/OfferToExchangeUSDollarsForRCredits.test', 'rsms', 0);
INSERT INTO `registry` VALUES('rsmsOpenAnAccountForTheCaller', 'class', 'rcredits/rsms/tests/OpenAnAccountForTheCaller.test', 'rsms', 0);
INSERT INTO `registry` VALUES('rsmsTransact', 'class', 'rcredits/rsms/tests/Transact.test', 'rsms', 0);
INSERT INTO `registry` VALUES('rsmsUndo', 'class', 'rcredits/rsms/tests/Undo.test', 'rsms', 0);
INSERT INTO `registry` VALUES('rwebmulti', 'class', 'rcredits/rweb/tests/multi.test', 'rweb', 0);
INSERT INTO `registry` VALUES('rwebRelations', 'class', 'rcredits/rweb/tests/Relations.test', 'rweb', 0);
INSERT INTO `registry` VALUES('rwebSignup', 'class', 'rcredits/rweb/tests/Signup.test', 'rweb', 0);
INSERT INTO `registry` VALUES('rwebSummary', 'class', 'rcredits/rweb/tests/Summary.test', 'rweb', 0);
INSERT INTO `registry` VALUES('rwebTransact', 'class', 'rcredits/rweb/tests/Transact.test', 'rweb', 0);
INSERT INTO `registry` VALUES('rwebTransactions', 'class', 'rcredits/rweb/tests/Transactions.test', 'rweb', 0);
INSERT INTO `registry` VALUES('SchemaCache', 'class', 'includes/bootstrap.inc', '', 0);
INSERT INTO `registry` VALUES('SelectQuery', 'class', 'includes/database/select.inc', '', 0);
INSERT INTO `registry` VALUES('SelectQuery_pgsql', 'class', 'includes/database/pgsql/select.inc', '', 0);
INSERT INTO `registry` VALUES('SelectQuery_sqlite', 'class', 'includes/database/sqlite/select.inc', '', 0);
INSERT INTO `registry` VALUES('SelectQueryExtender', 'class', 'includes/database/select.inc', '', 0);
INSERT INTO `registry` VALUES('SelectQueryInterface', 'interface', 'includes/database/select.inc', '', 0);
INSERT INTO `registry` VALUES('ShutdownFunctionsTest', 'class', 'modules/system/system.test', 'system', 0);
INSERT INTO `registry` VALUES('SiteMaintenanceTestCase', 'class', 'modules/system/system.test', 'system', 0);
INSERT INTO `registry` VALUES('SkipDotsRecursiveDirectoryIterator', 'class', 'includes/filetransfer/filetransfer.inc', '', 0);
INSERT INTO `registry` VALUES('StreamWrapperInterface', 'interface', 'includes/stream_wrappers.inc', '', 0);
INSERT INTO `registry` VALUES('SummaryLengthTestCase', 'class', 'modules/node/node.test', 'node', 0);
INSERT INTO `registry` VALUES('SystemAdminTestCase', 'class', 'modules/system/system.test', 'system', 0);
INSERT INTO `registry` VALUES('SystemAuthorizeCase', 'class', 'modules/system/system.test', 'system', 0);
INSERT INTO `registry` VALUES('SystemBlockTestCase', 'class', 'modules/system/system.test', 'system', 0);
INSERT INTO `registry` VALUES('SystemIndexPhpTest', 'class', 'modules/system/system.test', 'system', 0);
INSERT INTO `registry` VALUES('SystemInfoAlterTestCase', 'class', 'modules/system/system.test', 'system', 0);
INSERT INTO `registry` VALUES('SystemMainContentFallback', 'class', 'modules/system/system.test', 'system', 0);
INSERT INTO `registry` VALUES('SystemQueue', 'class', 'modules/system/system.queue.inc', 'system', 0);
INSERT INTO `registry` VALUES('SystemThemeFunctionalTest', 'class', 'modules/system/system.test', 'system', 0);
INSERT INTO `registry` VALUES('SystemValidTokenTest', 'class', 'modules/system/system.test', 'system', 0);
INSERT INTO `registry` VALUES('TableSort', 'class', 'includes/tablesort.inc', '', 0);
INSERT INTO `registry` VALUES('TestingMailSystem', 'class', 'modules/system/system.mail.inc', 'system', 0);
INSERT INTO `registry` VALUES('TextFieldTestCase', 'class', 'modules/field/modules/text/text.test', 'text', 0);
INSERT INTO `registry` VALUES('TextSummaryTestCase', 'class', 'modules/field/modules/text/text.test', 'text', 0);
INSERT INTO `registry` VALUES('TextTranslationTestCase', 'class', 'modules/field/modules/text/text.test', 'text', 0);
INSERT INTO `registry` VALUES('ThemeRegistry', 'class', 'includes/theme.inc', '', 0);
INSERT INTO `registry` VALUES('ThemeUpdater', 'class', 'modules/system/system.updater.inc', 'system', 0);
INSERT INTO `registry` VALUES('TokenReplaceTestCase', 'class', 'modules/system/system.test', 'system', 0);
INSERT INTO `registry` VALUES('TokenScanTest', 'class', 'modules/system/system.test', 'system', 0);
INSERT INTO `registry` VALUES('TruncateQuery', 'class', 'includes/database/query.inc', '', 0);
INSERT INTO `registry` VALUES('TruncateQuery_mysql', 'class', 'includes/database/mysql/query.inc', '', 0);
INSERT INTO `registry` VALUES('TruncateQuery_sqlite', 'class', 'includes/database/sqlite/query.inc', '', 0);
INSERT INTO `registry` VALUES('UpdateCoreTestCase', 'class', 'modules/update/update.test', 'update', 0);
INSERT INTO `registry` VALUES('UpdateCoreUnitTestCase', 'class', 'modules/update/update.test', 'update', 0);
INSERT INTO `registry` VALUES('UpdateQuery', 'class', 'includes/database/query.inc', '', 0);
INSERT INTO `registry` VALUES('UpdateQuery_pgsql', 'class', 'includes/database/pgsql/query.inc', '', 0);
INSERT INTO `registry` VALUES('UpdateQuery_sqlite', 'class', 'includes/database/sqlite/query.inc', '', 0);
INSERT INTO `registry` VALUES('Updater', 'class', 'includes/updater.inc', '', 0);
INSERT INTO `registry` VALUES('UpdaterException', 'class', 'includes/updater.inc', '', 0);
INSERT INTO `registry` VALUES('UpdaterFileTransferException', 'class', 'includes/updater.inc', '', 0);
INSERT INTO `registry` VALUES('UpdateScriptFunctionalTest', 'class', 'modules/system/system.test', 'system', 0);
INSERT INTO `registry` VALUES('UpdateTestContribCase', 'class', 'modules/update/update.test', 'update', 0);
INSERT INTO `registry` VALUES('UpdateTestHelper', 'class', 'modules/update/update.test', 'update', 0);
INSERT INTO `registry` VALUES('UpdateTestUploadCase', 'class', 'modules/update/update.test', 'update', 0);

-- --------------------------------------------------------

--
-- Table structure for table `registry_file`
--

CREATE TABLE `registry_file` (
  `filename` varchar(255) NOT NULL COMMENT 'Path to the file.',
  `hash` varchar(64) NOT NULL COMMENT 'sha-256 hash of the file’s contents when last parsed.'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Files parsed to build the registry.' ROW_FORMAT=DYNAMIC;

--
-- Dumping data for table `registry_file`
--

INSERT INTO `registry_file` VALUES('includes/actions.inc', 'f36b066681463c7dfe189e0430cb1a89bf66f7e228cbb53cdfcd93987193f759');
INSERT INTO `registry_file` VALUES('includes/ajax.inc', 'f5d608554c6b42b976d6a97e1efffe53c657e9fbb77eabb858935bfdf4276491');
INSERT INTO `registry_file` VALUES('includes/archiver.inc', 'bdbb21b712a62f6b913590b609fd17cd9f3c3b77c0d21f68e71a78427ed2e3e9');
INSERT INTO `registry_file` VALUES('includes/authorize.inc', '6d64d8c21aa01eb12fc29918732e4df6b871ed06e5d41373cb95c197ed661d13');
INSERT INTO `registry_file` VALUES('includes/batch.inc', '059da9e36e1f3717f27840aae73f10dea7d6c8daf16f6520401cc1ca3b4c0388');
INSERT INTO `registry_file` VALUES('includes/batch.queue.inc', '554b2e92e1dad0f7fd5a19cb8dff7e109f10fbe2441a5692d076338ec908de0f');
INSERT INTO `registry_file` VALUES('includes/bootstrap.inc', '80e808902f62fc0417227c40eac66b81fd6f692e85ab2587bde9ef5ecc0f3ba9');
INSERT INTO `registry_file` VALUES('includes/cache-install.inc', 'e7ed123c5805703c84ad2cce9c1ca46b3ce8caeeea0d8ef39a3024a4ab95fa0e');
INSERT INTO `registry_file` VALUES('includes/cache.inc', 'd01e10e4c18010b6908026f3d71b72717e3272cfb91a528490eba7f339f8dd1b');
INSERT INTO `registry_file` VALUES('includes/common.inc', 'a535d0b08fca17e3dac1416f31d7a5519433d1aaa02b55eebc06ecceafc84a61');
INSERT INTO `registry_file` VALUES('includes/database/database.inc', '24afaff6e1026bfe315205212cba72951240a16154250e405c4c64724e6e07cc');
INSERT INTO `registry_file` VALUES('includes/database/log.inc', '9feb5a17ae2fabcf26a96d2a634ba73da501f7bcfc3599a693d916a6971d00d1');
INSERT INTO `registry_file` VALUES('includes/database/mysql/database.inc', 'd62a2d8ca103cb3b085e7f8b894a7db14c02f20d0b1ed0bd32f6534a45b4527f');
INSERT INTO `registry_file` VALUES('includes/database/mysql/install.inc', '6ae316941f771732fbbabed7e1d6b4cbb41b1f429dd097d04b3345aa15e461a0');
INSERT INTO `registry_file` VALUES('includes/database/mysql/query.inc', '0212a871646c223bf77aa26b945c77a8974855373967b5fb9fdc09f8a1de88a6');
INSERT INTO `registry_file` VALUES('includes/database/mysql/schema.inc', '6f43ac87508f868fe38ee09994fc18d69915bada0237f8ac3b717cafe8f22c6b');
INSERT INTO `registry_file` VALUES('includes/database/prefetch.inc', 'b5b207a66a69ecb52ee4f4459af16a7b5eabedc87254245f37cc33bebb61c0fb');
INSERT INTO `registry_file` VALUES('includes/database/query.inc', '9171653e9710c6c0d20cff865fdead5a580367137ad4cdf81059ecc2eea61c74');
INSERT INTO `registry_file` VALUES('includes/database/schema.inc', 'a98b69d33975e75f7d99cb85b20c36b7fc10e35a588e07b20c1b37500f5876ca');
INSERT INTO `registry_file` VALUES('includes/database/select.inc', '5e9cdc383564ba86cb9dcad0046990ce15415a3000e4f617d6e0f30a205b852c');
INSERT INTO `registry_file` VALUES('includes/date.inc', '18c047be64f201e16d189f1cc47ed9dcf0a145151b1ee187e90511b24e5d2b36');
INSERT INTO `registry_file` VALUES('includes/entity.inc', '3080fe3c30991a48f1f314a60d02e841d263a8f222337e5bde3be61afe41ee7a');
INSERT INTO `registry_file` VALUES('includes/errors.inc', 'c8bda5b8fb4062823237d9b9ced5fb518d0a61b8ae7cee7e19ef0eba837e3d69');
INSERT INTO `registry_file` VALUES('includes/file.inc', 'cf1de474b1c36b8df3254730754cd8e747c2e9daaa3dc4df6eddd7bc2b870b43');
INSERT INTO `registry_file` VALUES('includes/file.mimetypes.inc', '33266e837f4ce076378e7e8cef6c5af46446226ca4259f83e13f605856a7f147');
INSERT INTO `registry_file` VALUES('includes/filetransfer/filetransfer.inc', 'fdea8ae48345ec91885ac48a9bc53daf87616271472bb7c29b7e3ce219b22034');
INSERT INTO `registry_file` VALUES('includes/filetransfer/ftp.inc', '51eb119b8e1221d598ffa6cc46c8a322aa77b49a3d8879f7fb38b7221cf7e06d');
INSERT INTO `registry_file` VALUES('includes/filetransfer/local.inc', '7cbfdb46abbdf539640db27e66fb30e5265128f31002bd0dfc3af16ae01a9492');
INSERT INTO `registry_file` VALUES('includes/filetransfer/ssh.inc', '92f1232158cb32ab04cbc93ae38ad3af04796e18f66910a9bc5ca8e437f06891');
INSERT INTO `registry_file` VALUES('includes/form.inc', '0dd082d8ca8fa99e93c33b79072efe1823a45ee8d7141ce93043c5d111136a41');
INSERT INTO `registry_file` VALUES('includes/graph.inc', '8e0e313a8bb33488f371df11fc1b58d7cf80099b886cd1003871e2c896d1b536');
INSERT INTO `registry_file` VALUES('includes/image.inc', 'bcdc7e1599c02227502b9d0fe36eeb2b529b130a392bc709eb737647bd361826');
INSERT INTO `registry_file` VALUES('includes/install.core.inc', 'a0585c85002e6f3d702dc505584f48b55bc13e24bee749bfe5b718fbce4847e1');
INSERT INTO `registry_file` VALUES('includes/install.inc', '480c3cfd065d3ec00f4465e1b0a0d55d6a8927e78fd6774001c30163a5c648e3');
INSERT INTO `registry_file` VALUES('includes/iso.inc', '0ce4c225edcfa9f037703bc7dd09d4e268a69bcc90e55da0a3f04c502bd2f349');
INSERT INTO `registry_file` VALUES('includes/json-encode.inc', '02a822a652d00151f79db9aa9e171c310b69b93a12f549bc2ce00533a8efa14e');
INSERT INTO `registry_file` VALUES('includes/language.inc', '4dd521af07e0ca7bf97ff145f4bd3a218acf0d8b94964e72f11212bb8af8d66e');
INSERT INTO `registry_file` VALUES('includes/locale.inc', 'b250f375b93ffe3749f946e0ad475065c914af23e388d68e5c5df161590f086a');
INSERT INTO `registry_file` VALUES('includes/lock.inc', 'a181c8bd4f88d292a0a73b9f1fbd727e3314f66ec3631f288e6b9a54ba2b70fa');
INSERT INTO `registry_file` VALUES('includes/mail.inc', 'd9fb2b99025745cbb73ebcfc7ac12df100508b9273ce35c433deacf12dd6a13a');
INSERT INTO `registry_file` VALUES('includes/menu.inc', 'c9ff3c7db04b7e01d0d19b5e47d9fb209799f2ae6584167235b957d22542e526');
INSERT INTO `registry_file` VALUES('includes/module.inc', 'f63ab8cec01f932d7abfc2d09d91ba322e333f4ff447088ab0db4d16b5d9f676');
INSERT INTO `registry_file` VALUES('includes/pager.inc', '6f9494b85c07a2cc3be4e54aff2d2757485238c476a7da084d25bde1d88be6d8');
INSERT INTO `registry_file` VALUES('includes/password.inc', 'fd9a1c94fe5a0fa7c7049a2435c7280b1d666b2074595010e3c492dd15712775');
INSERT INTO `registry_file` VALUES('includes/path.inc', '74bf05f3c68b0218730abf3e539fcf08b271959c8f4611940d05124f34a6a66f');
INSERT INTO `registry_file` VALUES('includes/registry.inc', 'c225de772f86eebd21b0b52fa8fcc6671e05fa2374cedb3164f7397f27d3c88d');
INSERT INTO `registry_file` VALUES('includes/session.inc', '7548621ae4c273179a76eba41aa58b740100613bc015ad388a5c30132b61e34b');
INSERT INTO `registry_file` VALUES('includes/stream_wrappers.inc', '4f1feb774a8dbc04ca382fa052f59e58039c7261625f3df29987d6b31f08d92d');
INSERT INTO `registry_file` VALUES('includes/tablesort.inc', '2d88768a544829595dd6cda2a5eb008bedb730f36bba6dfe005d9ddd999d5c0f');
INSERT INTO `registry_file` VALUES('includes/theme.inc', 'ab2a805bb52a54dc762f314bbba6b55b959734a87e8f96119435d08b08e6fe1f');
INSERT INTO `registry_file` VALUES('includes/theme.maintenance.inc', '39f068b3eee4d10a90d6aa3c86db587b6d25844c2919d418d34d133cfe330f5a');
INSERT INTO `registry_file` VALUES('includes/token.inc', '5e7898cd78689e2c291ed3cd8f41c032075656896f1db57e49217aac19ae0428');
INSERT INTO `registry_file` VALUES('includes/unicode.entities.inc', '2b858138596d961fbaa4c6e3986e409921df7f76b6ee1b109c4af5970f1e0f54');
INSERT INTO `registry_file` VALUES('includes/unicode.inc', 'e18772dafe0f80eb139fcfc582fef1704ba9f730647057d4f4841d6a6e4066ca');
INSERT INTO `registry_file` VALUES('includes/update.inc', '177ce24362efc7f28b384c90a09c3e485396bbd18c3721d4b21e57dd1733bd92');
INSERT INTO `registry_file` VALUES('includes/updater.inc', 'd2da0e74ed86e93c209f16069f3d32e1a134ceb6c06a0044f78e841a1b54e380');
INSERT INTO `registry_file` VALUES('includes/utility.inc', '3458fd2b55ab004dd0cc529b8e58af12916e8bd36653b072bdd820b26b907ed5');
INSERT INTO `registry_file` VALUES('includes/xmlrpc.inc', 'ea24176ec445c440ba0c825fc7b04a31b440288df8ef02081560dc418e34e659');
INSERT INTO `registry_file` VALUES('includes/xmlrpcs.inc', '741aa8d6fcc6c45a9409064f52351f7999b7c702d73def8da44de2567946598a');
INSERT INTO `registry_file` VALUES('modules/block/block.test', 'df1b364688b46345523dfcb95c0c48352d6a4edbc66597890d29b9b0d7866e86');
INSERT INTO `registry_file` VALUES('modules/field/field.attach.inc', '2df4687b5ec078c4893dc1fea514f67524fd5293de717b9e05caf977e5ae2327');
INSERT INTO `registry_file` VALUES('modules/field/field.info.class.inc', 'a6f2f418552dba0e03f57ee812a6f0f63bbfe4bf81fe805d51ecec47ef84b845');
INSERT INTO `registry_file` VALUES('modules/field/field.module', '2ec1a3ec060504467c3065426a5a1eca8e2c894cb4d4480616bca60fe4b2faf2');
INSERT INTO `registry_file` VALUES('modules/system/system.archiver.inc', 'faa849f3e646a910ab82fd6c8bbf0a4e6b8c60725d7ba81ec0556bd716616cd1');
INSERT INTO `registry_file` VALUES('modules/system/system.mail.inc', 'd31e1769f5defbe5f27dc68f641ab80fb8d3de92f6e895f4c654ec05fc7e5f0f');
INSERT INTO `registry_file` VALUES('modules/system/system.queue.inc', 'ef00fd41ca86de386fa134d5bc1d816f9af550cf0e1334a5c0ade3119688ca3c');
INSERT INTO `registry_file` VALUES('modules/system/system.tar.inc', '8a31d91f7b3cd7eac25b3fa46e1ed9a8527c39718ba76c3f8c0bbbeaa3aa4086');
INSERT INTO `registry_file` VALUES('modules/system/system.test', 'ad3c68f2cacfe6a99c065edc9aca05a22bdbc74ff6158e9918255b4633134ab4');
INSERT INTO `registry_file` VALUES('modules/system/system.updater.inc', '338cf14cb691ba16ee551b3b9e0fa4f579a2f25c964130658236726d17563b6a');

-- --------------------------------------------------------

--
-- Table structure for table `r_areas`
--

CREATE TABLE `r_areas` (
  `area_code` char(3) NOT NULL DEFAULT '' COMMENT 'telephone area code',
  `region` varchar(24) DEFAULT NULL COMMENT 'state, province, or territory'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC;

--
-- Dumping data for table `r_areas`
--

INSERT INTO `r_areas` VALUES('201', 'NJ');
INSERT INTO `r_areas` VALUES('202', 'DC');
INSERT INTO `r_areas` VALUES('203', 'CT');
INSERT INTO `r_areas` VALUES('204', 'MB');
INSERT INTO `r_areas` VALUES('205', 'AL');
INSERT INTO `r_areas` VALUES('206', 'WA');
INSERT INTO `r_areas` VALUES('207', 'ME');
INSERT INTO `r_areas` VALUES('208', 'ID');
INSERT INTO `r_areas` VALUES('209', 'CA');
INSERT INTO `r_areas` VALUES('210', 'TX');
INSERT INTO `r_areas` VALUES('212', 'NY');
INSERT INTO `r_areas` VALUES('213', 'CA');
INSERT INTO `r_areas` VALUES('214', 'TX');
INSERT INTO `r_areas` VALUES('215', 'PA');
INSERT INTO `r_areas` VALUES('216', 'OH');
INSERT INTO `r_areas` VALUES('217', 'IL');
INSERT INTO `r_areas` VALUES('218', 'MN');
INSERT INTO `r_areas` VALUES('219', 'IN');
INSERT INTO `r_areas` VALUES('224', 'IL');
INSERT INTO `r_areas` VALUES('225', 'LA');
INSERT INTO `r_areas` VALUES('226', 'ON');
INSERT INTO `r_areas` VALUES('228', 'MS');
INSERT INTO `r_areas` VALUES('229', 'GA');
INSERT INTO `r_areas` VALUES('231', 'MI');
INSERT INTO `r_areas` VALUES('234', 'OH');
INSERT INTO `r_areas` VALUES('239', 'FL');
INSERT INTO `r_areas` VALUES('240', 'MD');
INSERT INTO `r_areas` VALUES('242', 'Bahamas');
INSERT INTO `r_areas` VALUES('246', 'Barbados');
INSERT INTO `r_areas` VALUES('248', 'MI');
INSERT INTO `r_areas` VALUES('250', 'BC');
INSERT INTO `r_areas` VALUES('251', 'AL');
INSERT INTO `r_areas` VALUES('252', 'NC');
INSERT INTO `r_areas` VALUES('253', 'WA');
INSERT INTO `r_areas` VALUES('254', 'TX');
INSERT INTO `r_areas` VALUES('256', 'AL');
INSERT INTO `r_areas` VALUES('260', 'IN');
INSERT INTO `r_areas` VALUES('262', 'WI');
INSERT INTO `r_areas` VALUES('264', 'Anguilla');
INSERT INTO `r_areas` VALUES('267', 'PA');
INSERT INTO `r_areas` VALUES('268', 'Antigua and Barbuda');
INSERT INTO `r_areas` VALUES('269', 'MI');
INSERT INTO `r_areas` VALUES('270', 'KY');
INSERT INTO `r_areas` VALUES('276', 'VA');
INSERT INTO `r_areas` VALUES('281', 'TX');
INSERT INTO `r_areas` VALUES('284', 'British Virgin Islands');
INSERT INTO `r_areas` VALUES('289', 'ON');
INSERT INTO `r_areas` VALUES('301', 'MD');
INSERT INTO `r_areas` VALUES('302', 'DE');
INSERT INTO `r_areas` VALUES('303', 'CO');
INSERT INTO `r_areas` VALUES('304', 'WV');
INSERT INTO `r_areas` VALUES('305', 'FL');
INSERT INTO `r_areas` VALUES('306', 'SK');
INSERT INTO `r_areas` VALUES('307', 'WY');
INSERT INTO `r_areas` VALUES('308', 'NE');
INSERT INTO `r_areas` VALUES('309', 'IL');
INSERT INTO `r_areas` VALUES('310', 'CA');
INSERT INTO `r_areas` VALUES('312', 'IL');
INSERT INTO `r_areas` VALUES('313', 'MI');
INSERT INTO `r_areas` VALUES('314', 'MO');
INSERT INTO `r_areas` VALUES('315', 'NY');
INSERT INTO `r_areas` VALUES('316', 'KS');
INSERT INTO `r_areas` VALUES('317', 'IN');
INSERT INTO `r_areas` VALUES('318', 'LA');
INSERT INTO `r_areas` VALUES('319', 'IA');
INSERT INTO `r_areas` VALUES('320', 'MN');
INSERT INTO `r_areas` VALUES('321', 'FL');
INSERT INTO `r_areas` VALUES('323', 'CA');
INSERT INTO `r_areas` VALUES('325', 'TX');
INSERT INTO `r_areas` VALUES('330', 'OH');
INSERT INTO `r_areas` VALUES('334', 'AL');
INSERT INTO `r_areas` VALUES('336', 'NC');
INSERT INTO `r_areas` VALUES('337', 'LA');
INSERT INTO `r_areas` VALUES('339', 'MA');
INSERT INTO `r_areas` VALUES('340', 'USVI');
INSERT INTO `r_areas` VALUES('345', 'Cayman Islands');
INSERT INTO `r_areas` VALUES('347', 'NY');
INSERT INTO `r_areas` VALUES('351', 'MA');
INSERT INTO `r_areas` VALUES('352', 'FL');
INSERT INTO `r_areas` VALUES('360', 'WA');
INSERT INTO `r_areas` VALUES('361', 'TX');
INSERT INTO `r_areas` VALUES('386', 'FL');
INSERT INTO `r_areas` VALUES('401', 'RI');
INSERT INTO `r_areas` VALUES('402', 'NE');
INSERT INTO `r_areas` VALUES('403', 'AB');
INSERT INTO `r_areas` VALUES('404', 'GA');
INSERT INTO `r_areas` VALUES('405', 'OK');
INSERT INTO `r_areas` VALUES('406', 'MT');
INSERT INTO `r_areas` VALUES('407', 'FL');
INSERT INTO `r_areas` VALUES('408', 'CA');
INSERT INTO `r_areas` VALUES('409', 'TX');
INSERT INTO `r_areas` VALUES('410', 'MD');
INSERT INTO `r_areas` VALUES('412', 'PA');
INSERT INTO `r_areas` VALUES('413', 'MA');
INSERT INTO `r_areas` VALUES('414', 'WI');
INSERT INTO `r_areas` VALUES('415', 'CA');
INSERT INTO `r_areas` VALUES('416', 'ON');
INSERT INTO `r_areas` VALUES('417', 'MO');
INSERT INTO `r_areas` VALUES('418', 'QC');
INSERT INTO `r_areas` VALUES('419', 'OH');
INSERT INTO `r_areas` VALUES('423', 'TN');
INSERT INTO `r_areas` VALUES('424', 'CA');
INSERT INTO `r_areas` VALUES('425', 'WA');
INSERT INTO `r_areas` VALUES('430', 'TX');
INSERT INTO `r_areas` VALUES('432', 'TX');
INSERT INTO `r_areas` VALUES('434', 'VA');
INSERT INTO `r_areas` VALUES('435', 'UT');
INSERT INTO `r_areas` VALUES('438', 'QC');
INSERT INTO `r_areas` VALUES('440', 'OH');
INSERT INTO `r_areas` VALUES('441', 'Bermuda');
INSERT INTO `r_areas` VALUES('443', 'MD');
INSERT INTO `r_areas` VALUES('450', 'QC');
INSERT INTO `r_areas` VALUES('456', 'Reserved');
INSERT INTO `r_areas` VALUES('469', 'TX');
INSERT INTO `r_areas` VALUES('473', 'Grenada');
INSERT INTO `r_areas` VALUES('478', 'GA');
INSERT INTO `r_areas` VALUES('479', 'AR');
INSERT INTO `r_areas` VALUES('480', 'AZ');
INSERT INTO `r_areas` VALUES('484', 'PA');
INSERT INTO `r_areas` VALUES('500', 'Reserved');
INSERT INTO `r_areas` VALUES('501', 'AR');
INSERT INTO `r_areas` VALUES('502', 'KY');
INSERT INTO `r_areas` VALUES('503', 'OR');
INSERT INTO `r_areas` VALUES('504', 'LA');
INSERT INTO `r_areas` VALUES('505', 'NM');
INSERT INTO `r_areas` VALUES('506', 'NB');
INSERT INTO `r_areas` VALUES('507', 'MN');
INSERT INTO `r_areas` VALUES('508', 'MA');
INSERT INTO `r_areas` VALUES('509', 'WA');
INSERT INTO `r_areas` VALUES('510', 'CA');
INSERT INTO `r_areas` VALUES('512', 'TX');
INSERT INTO `r_areas` VALUES('513', 'OH');
INSERT INTO `r_areas` VALUES('514', 'QC');
INSERT INTO `r_areas` VALUES('515', 'IA');
INSERT INTO `r_areas` VALUES('516', 'NY');
INSERT INTO `r_areas` VALUES('517', 'MI');
INSERT INTO `r_areas` VALUES('518', 'NY');
INSERT INTO `r_areas` VALUES('519', 'ON');
INSERT INTO `r_areas` VALUES('520', 'AZ');
INSERT INTO `r_areas` VALUES('530', 'CA');
INSERT INTO `r_areas` VALUES('540', 'VA');
INSERT INTO `r_areas` VALUES('541', 'OR');
INSERT INTO `r_areas` VALUES('551', 'NJ');
INSERT INTO `r_areas` VALUES('559', 'CA');
INSERT INTO `r_areas` VALUES('561', 'FL');
INSERT INTO `r_areas` VALUES('562', 'CA');
INSERT INTO `r_areas` VALUES('563', 'IA');
INSERT INTO `r_areas` VALUES('567', 'OH');
INSERT INTO `r_areas` VALUES('570', 'PA');
INSERT INTO `r_areas` VALUES('571', 'VA');
INSERT INTO `r_areas` VALUES('573', 'MO');
INSERT INTO `r_areas` VALUES('574', 'IN');
INSERT INTO `r_areas` VALUES('580', 'OK');
INSERT INTO `r_areas` VALUES('585', 'NY');
INSERT INTO `r_areas` VALUES('586', 'MI');
INSERT INTO `r_areas` VALUES('600', 'Reserved');
INSERT INTO `r_areas` VALUES('601', 'MS');
INSERT INTO `r_areas` VALUES('602', 'AZ');
INSERT INTO `r_areas` VALUES('603', 'NH');
INSERT INTO `r_areas` VALUES('604', 'BC');
INSERT INTO `r_areas` VALUES('605', 'SD');
INSERT INTO `r_areas` VALUES('606', 'KY');
INSERT INTO `r_areas` VALUES('607', 'NY');
INSERT INTO `r_areas` VALUES('608', 'WI');
INSERT INTO `r_areas` VALUES('609', 'NJ');
INSERT INTO `r_areas` VALUES('610', 'PA');
INSERT INTO `r_areas` VALUES('612', 'MN');
INSERT INTO `r_areas` VALUES('613', 'ON');
INSERT INTO `r_areas` VALUES('614', 'OH');
INSERT INTO `r_areas` VALUES('615', 'TN');
INSERT INTO `r_areas` VALUES('616', 'MI');
INSERT INTO `r_areas` VALUES('617', 'MA');
INSERT INTO `r_areas` VALUES('618', 'IL');
INSERT INTO `r_areas` VALUES('619', 'CA');
INSERT INTO `r_areas` VALUES('620', 'KS');
INSERT INTO `r_areas` VALUES('623', 'AZ');
INSERT INTO `r_areas` VALUES('626', 'CA');
INSERT INTO `r_areas` VALUES('630', 'IL');
INSERT INTO `r_areas` VALUES('631', 'NY');
INSERT INTO `r_areas` VALUES('636', 'MO');
INSERT INTO `r_areas` VALUES('641', 'IA');
INSERT INTO `r_areas` VALUES('646', 'NY');
INSERT INTO `r_areas` VALUES('647', 'ON');
INSERT INTO `r_areas` VALUES('649', 'Turks & Caicos Islands');
INSERT INTO `r_areas` VALUES('650', 'CA');
INSERT INTO `r_areas` VALUES('651', 'MN');
INSERT INTO `r_areas` VALUES('660', 'MO');
INSERT INTO `r_areas` VALUES('661', 'CA');
INSERT INTO `r_areas` VALUES('662', 'MS');
INSERT INTO `r_areas` VALUES('664', 'Montserrat');
INSERT INTO `r_areas` VALUES('670', 'MP');
INSERT INTO `r_areas` VALUES('671', 'GU');
INSERT INTO `r_areas` VALUES('678', 'GA');
INSERT INTO `r_areas` VALUES('682', 'TX');
INSERT INTO `r_areas` VALUES('684', 'AS');
INSERT INTO `r_areas` VALUES('700', 'Reserved');
INSERT INTO `r_areas` VALUES('701', 'ND');
INSERT INTO `r_areas` VALUES('702', 'NV');
INSERT INTO `r_areas` VALUES('703', 'VA');
INSERT INTO `r_areas` VALUES('704', 'NC');
INSERT INTO `r_areas` VALUES('705', 'ON');
INSERT INTO `r_areas` VALUES('706', 'GA');
INSERT INTO `r_areas` VALUES('707', 'CA');
INSERT INTO `r_areas` VALUES('708', 'IL');
INSERT INTO `r_areas` VALUES('709', 'NL');
INSERT INTO `r_areas` VALUES('710', 'US Government');
INSERT INTO `r_areas` VALUES('712', 'IA');
INSERT INTO `r_areas` VALUES('713', 'TX');
INSERT INTO `r_areas` VALUES('714', 'CA');
INSERT INTO `r_areas` VALUES('715', 'WI');
INSERT INTO `r_areas` VALUES('716', 'NY');
INSERT INTO `r_areas` VALUES('717', 'PA');
INSERT INTO `r_areas` VALUES('718', 'NY');
INSERT INTO `r_areas` VALUES('719', 'CO');
INSERT INTO `r_areas` VALUES('720', 'CO');
INSERT INTO `r_areas` VALUES('724', 'PA');
INSERT INTO `r_areas` VALUES('727', 'FL');
INSERT INTO `r_areas` VALUES('731', 'TN');
INSERT INTO `r_areas` VALUES('732', 'NJ');
INSERT INTO `r_areas` VALUES('734', 'MI');
INSERT INTO `r_areas` VALUES('740', 'OH');
INSERT INTO `r_areas` VALUES('754', 'FL');
INSERT INTO `r_areas` VALUES('757', 'VA');
INSERT INTO `r_areas` VALUES('758', 'St. Lucia');
INSERT INTO `r_areas` VALUES('760', 'CA');
INSERT INTO `r_areas` VALUES('762', 'GA');
INSERT INTO `r_areas` VALUES('763', 'MN');
INSERT INTO `r_areas` VALUES('765', 'IN');
INSERT INTO `r_areas` VALUES('767', 'Dominica');
INSERT INTO `r_areas` VALUES('769', 'MS');
INSERT INTO `r_areas` VALUES('770', 'GA');
INSERT INTO `r_areas` VALUES('772', 'FL');
INSERT INTO `r_areas` VALUES('773', 'IL');
INSERT INTO `r_areas` VALUES('774', 'MA');
INSERT INTO `r_areas` VALUES('775', 'NV');
INSERT INTO `r_areas` VALUES('778', 'BC');
INSERT INTO `r_areas` VALUES('780', 'AB');
INSERT INTO `r_areas` VALUES('781', 'MA');
INSERT INTO `r_areas` VALUES('784', 'St. Vincent & Grenadines');
INSERT INTO `r_areas` VALUES('785', 'KS');
INSERT INTO `r_areas` VALUES('786', 'FL');
INSERT INTO `r_areas` VALUES('787', 'PR');
INSERT INTO `r_areas` VALUES('800', 'Toll Free');
INSERT INTO `r_areas` VALUES('801', 'UT');
INSERT INTO `r_areas` VALUES('802', 'VT');
INSERT INTO `r_areas` VALUES('803', 'SC');
INSERT INTO `r_areas` VALUES('804', 'VA');
INSERT INTO `r_areas` VALUES('805', 'CA');
INSERT INTO `r_areas` VALUES('806', 'TX');
INSERT INTO `r_areas` VALUES('807', 'ON');
INSERT INTO `r_areas` VALUES('808', 'HI');
INSERT INTO `r_areas` VALUES('809', 'Dominican Republic');
INSERT INTO `r_areas` VALUES('810', 'MI');
INSERT INTO `r_areas` VALUES('812', 'IN');
INSERT INTO `r_areas` VALUES('813', 'FL');
INSERT INTO `r_areas` VALUES('814', 'PA');
INSERT INTO `r_areas` VALUES('815', 'IL');
INSERT INTO `r_areas` VALUES('816', 'MO');
INSERT INTO `r_areas` VALUES('817', 'TX');
INSERT INTO `r_areas` VALUES('818', 'CA');
INSERT INTO `r_areas` VALUES('819', 'QC');
INSERT INTO `r_areas` VALUES('828', 'NC');
INSERT INTO `r_areas` VALUES('829', 'Dominican Republic');
INSERT INTO `r_areas` VALUES('830', 'TX');
INSERT INTO `r_areas` VALUES('831', 'CA');
INSERT INTO `r_areas` VALUES('832', 'TX');
INSERT INTO `r_areas` VALUES('843', 'SC');
INSERT INTO `r_areas` VALUES('845', 'NY');
INSERT INTO `r_areas` VALUES('847', 'IL');
INSERT INTO `r_areas` VALUES('848', 'NJ');
INSERT INTO `r_areas` VALUES('850', 'FL');
INSERT INTO `r_areas` VALUES('856', 'NJ');
INSERT INTO `r_areas` VALUES('857', 'MA');
INSERT INTO `r_areas` VALUES('858', 'CA');
INSERT INTO `r_areas` VALUES('859', 'KY');
INSERT INTO `r_areas` VALUES('860', 'CT');
INSERT INTO `r_areas` VALUES('862', 'NJ');
INSERT INTO `r_areas` VALUES('863', 'FL');
INSERT INTO `r_areas` VALUES('864', 'SC');
INSERT INTO `r_areas` VALUES('865', 'TN');
INSERT INTO `r_areas` VALUES('866', 'Toll Free');
INSERT INTO `r_areas` VALUES('867', 'YT');
INSERT INTO `r_areas` VALUES('868', 'Trinidad & Tobago');
INSERT INTO `r_areas` VALUES('869', 'St. Kitts & Nevis');
INSERT INTO `r_areas` VALUES('870', 'AR');
INSERT INTO `r_areas` VALUES('876', 'Jamaica');
INSERT INTO `r_areas` VALUES('877', 'Toll Free');
INSERT INTO `r_areas` VALUES('878', 'PA');
INSERT INTO `r_areas` VALUES('888', 'Toll Free');
INSERT INTO `r_areas` VALUES('900', 'Toll Calls');
INSERT INTO `r_areas` VALUES('901', 'TN');
INSERT INTO `r_areas` VALUES('902', 'NS');
INSERT INTO `r_areas` VALUES('903', 'TX');
INSERT INTO `r_areas` VALUES('904', 'FL');
INSERT INTO `r_areas` VALUES('905', 'ON');
INSERT INTO `r_areas` VALUES('906', 'MI');
INSERT INTO `r_areas` VALUES('907', 'AK');
INSERT INTO `r_areas` VALUES('908', 'NJ');
INSERT INTO `r_areas` VALUES('909', 'CA');
INSERT INTO `r_areas` VALUES('910', 'NC');
INSERT INTO `r_areas` VALUES('912', 'GA');
INSERT INTO `r_areas` VALUES('913', 'KS');
INSERT INTO `r_areas` VALUES('914', 'NY');
INSERT INTO `r_areas` VALUES('915', 'TX');
INSERT INTO `r_areas` VALUES('916', 'CA');
INSERT INTO `r_areas` VALUES('917', 'NY');
INSERT INTO `r_areas` VALUES('918', 'OK');
INSERT INTO `r_areas` VALUES('919', 'NC');
INSERT INTO `r_areas` VALUES('920', 'WI');
INSERT INTO `r_areas` VALUES('925', 'CA');
INSERT INTO `r_areas` VALUES('928', 'AZ');
INSERT INTO `r_areas` VALUES('931', 'TN');
INSERT INTO `r_areas` VALUES('936', 'TX');
INSERT INTO `r_areas` VALUES('937', 'OH');
INSERT INTO `r_areas` VALUES('939', 'PR');
INSERT INTO `r_areas` VALUES('940', 'TX');
INSERT INTO `r_areas` VALUES('941', 'FL');
INSERT INTO `r_areas` VALUES('947', 'MI');
INSERT INTO `r_areas` VALUES('949', 'CA');
INSERT INTO `r_areas` VALUES('951', 'CA');
INSERT INTO `r_areas` VALUES('952', 'MN');
INSERT INTO `r_areas` VALUES('954', 'FL');
INSERT INTO `r_areas` VALUES('956', 'TX');
INSERT INTO `r_areas` VALUES('970', 'CO');
INSERT INTO `r_areas` VALUES('971', 'OR');
INSERT INTO `r_areas` VALUES('972', 'TX');
INSERT INTO `r_areas` VALUES('973', 'NJ');
INSERT INTO `r_areas` VALUES('978', 'MA');
INSERT INTO `r_areas` VALUES('979', 'TX');
INSERT INTO `r_areas` VALUES('980', 'NC');
INSERT INTO `r_areas` VALUES('985', 'LA');
INSERT INTO `r_areas` VALUES('989', 'MI');

-- --------------------------------------------------------

--
-- Table structure for table `r_bad`
--

CREATE TABLE `r_bad` (
  `qid` varchar(255) DEFAULT NULL COMMENT 'phoney customer qid',
  `code` varchar(255) DEFAULT NULL COMMENT 'phoney card security code',
  `created` int(11) NOT NULL COMMENT 'Unixtime record was created'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='lost, stolen, or faked rCard codes' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `r_ballots`
--

CREATE TABLE `r_ballots` (
  `id` bigint(20) NOT NULL COMMENT 'ballot record id',
  `question` bigint(20) NOT NULL DEFAULT 0 COMMENT 'question or proposal voted on by a particular voter',
  `voter` bigint(20) NOT NULL DEFAULT 0 COMMENT 'record id of voter whose ballot this is',
  `proxy` bigint(20) NOT NULL DEFAULT 0 COMMENT 'record id of voter who actually voted on behalf of the voter',
  `modified` int(11) NOT NULL DEFAULT 0 COMMENT 'date/time last modified',
  `created` int(11) NOT NULL DEFAULT 0 COMMENT 'date/time created'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='A votable question addressed by a particular voter' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `r_banks`
--

CREATE TABLE `r_banks` (
  `route` int(9) NOT NULL COMMENT 'routing number',
  `branch` tinyint(4) DEFAULT NULL COMMENT 'is this a branch office',
  `fedRoute` int(9) DEFAULT NULL COMMENT 'routing number of servicing Fed',
  `type` tinyint(4) DEFAULT NULL COMMENT 'bank type',
  `modified` char(6) DEFAULT NULL COMMENT 'date modified',
  `newRoute` int(9) DEFAULT NULL COMMENT 'new routing number',
  `name` varchar(36) DEFAULT NULL COMMENT 'bank name',
  `address` varchar(36) DEFAULT NULL COMMENT 'bank address',
  `city` varchar(20) DEFAULT NULL COMMENT 'bank city',
  `state` char(2) DEFAULT NULL COMMENT 'bank state',
  `zip` char(9) DEFAULT NULL COMMENT 'bank zipcode',
  `phone` char(10) DEFAULT NULL COMMENT 'bank phone',
  `status` char(1) DEFAULT NULL COMMENT 'status',
  `view` char(1) DEFAULT NULL COMMENT 'status'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Bank routing numbers' ROW_FORMAT=DYNAMIC;

--
-- Dumping data for table `r_banks`
--

INSERT INTO `r_banks` VALUES
(211870281, 0, 11000015, 1, '032608', 0, 'Greenfield Co-op Bank', '63 Fed St Box 1345', 'Greenfield', 'MA', '013010000', '4137720293', '1', '1');

-- --------------------------------------------------------

--
-- Table structure for table `r_boxes`
--

CREATE TABLE `r_boxes` (
  `id` bigint(20) NOT NULL COMMENT 'device record id',
  `channel` tinyint(4) DEFAULT NULL COMMENT 'channel',
  `code` varchar(255) DEFAULT NULL COMMENT 'device id',
  `boxnum` int(11) NOT NULL DEFAULT 0 COMMENT 'sequential device number for the account',
  `uid` bigint(20) DEFAULT NULL COMMENT 'account record id',
  `boxName` varchar(255) DEFAULT NULL COMMENT 'member''s chosen name for this device, for this account',
  `todo` longtext DEFAULT NULL COMMENT 'waiting for confirmation to complete this operation',
  `nonce` varchar(255) DEFAULT NULL COMMENT 'waiting for this nonce, for confirmation',
  `version` mediumint(9) DEFAULT NULL COMMENT 'latest software version on the device',
  `access` int(11) NOT NULL DEFAULT 0 COMMENT 'date/time last used',
  `created` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime record was created',
  `restricted` tinyint(4) DEFAULT 0 COMMENT 'permit no new users of this device'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Names for devices' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `r_changes`
--

CREATE TABLE `r_changes` (
  `id` bigint(20) NOT NULL COMMENT 'change record ID',
  `uid` bigint(20) DEFAULT NULL COMMENT 'id of user record to which this change applies',
  `created` int(11) DEFAULT NULL COMMENT 'Unix date and time that change was made',
  `field` varchar(40) DEFAULT NULL COMMENT 'name of the field that was changed',
  `oldValue` blob DEFAULT NULL COMMENT 'serialized old value, possibly encrypted',
  `newValue` blob DEFAULT NULL COMMENT 'serialized new value, possibly encrypted',
  `changedBy` bigint(20) DEFAULT NULL COMMENT 'uid of user who made the change'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Member record changes' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `r_countries`
--

CREATE TABLE `r_countries` (
  `id` int(10) UNSIGNED NOT NULL COMMENT 'Country Id',
  `name` varchar(64) DEFAULT NULL COMMENT 'Country Name',
  `iso_code` char(2) DEFAULT NULL COMMENT 'ISO Code',
  `country_code` varchar(4) DEFAULT NULL COMMENT 'National prefix to be used when dialing TO this country.',
  `address_format_id` int(10) UNSIGNED DEFAULT NULL COMMENT 'Foreign key to civicrm_address_format.id.',
  `idd_prefix` varchar(4) DEFAULT NULL COMMENT 'International direct dialing prefix from within the country TO another country',
  `ndd_prefix` varchar(4) DEFAULT NULL COMMENT 'Access prefix to call within a country to a different area',
  `region_id` int(10) UNSIGNED NOT NULL COMMENT 'Foreign key to civicrm_worldregion.id.',
  `is_province_abbreviated` tinyint(4) DEFAULT 0 COMMENT 'Should state/province be displayed as abbreviation?'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC;

--
-- Dumping data for table `r_countries`
--

INSERT INTO `r_countries` VALUES(1001, 'Afghanistan', 'AF', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1002, 'Albania', 'AL', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1003, 'Algeria', 'DZ', NULL, NULL, NULL, NULL, 3, 0);
INSERT INTO `r_countries` VALUES(1004, 'American Samoa', 'AS', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1005, 'Andorra', 'AD', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1006, 'Angola', 'AO', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1007, 'Anguilla', 'AI', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1008, 'Antarctica', 'AQ', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1009, 'Antigua and Barbuda', 'AG', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1010, 'Argentina', 'AR', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1011, 'Armenia', 'AM', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1012, 'Aruba', 'AW', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1013, 'Australia', 'AU', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1014, 'Austria', 'AT', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1015, 'Azerbaijan', 'AZ', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1016, 'Bahrain', 'BH', NULL, NULL, NULL, NULL, 3, 0);
INSERT INTO `r_countries` VALUES(1017, 'Bangladesh', 'BD', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1018, 'Barbados', 'BB', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1019, 'Belarus', 'BY', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1020, 'Belgium', 'BE', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1021, 'Belize', 'BZ', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1022, 'Benin', 'BJ', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1023, 'Bermuda', 'BM', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1024, 'Bhutan', 'BT', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1025, 'Bolivia', 'BO', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1026, 'Bosnia and Herzegovina', 'BA', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1027, 'Botswana', 'BW', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1028, 'Bouvet Island', 'BV', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1029, 'Brazil', 'BR', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1030, 'British Indian Ocean Territory', 'IO', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1031, 'Virgin Islands, British', 'VG', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1032, 'Brunei Darussalam', 'BN', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1033, 'Bulgaria', 'BG', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1034, 'Burkina Faso', 'BF', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1035, 'Myanmar', 'MM', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1036, 'Burundi', 'BI', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1037, 'Cambodia', 'KH', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1038, 'Cameroon', 'CM', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1039, 'Canada', 'CA', NULL, NULL, NULL, NULL, 2, 1);
INSERT INTO `r_countries` VALUES(1040, 'Cape Verde', 'CV', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1041, 'Cayman Islands', 'KY', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1042, 'Central African Republic', 'CF', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1043, 'Chad', 'TD', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1044, 'Chile', 'CL', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1045, 'China', 'CN', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1046, 'Christmas Island', 'CX', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1047, 'Cocos (Keeling) Islands', 'CC', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1048, 'Colombia', 'CO', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1049, 'Comoros', 'KM', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1050, 'Congo, The Democratic Republic of the', 'CD', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1051, 'Congo', 'CG', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1052, 'Cook Islands', 'CK', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1053, 'Costa Rica', 'CR', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1054, 'CÃ´te d\'Ivoire', 'CI', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1055, 'Croatia', 'HR', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1056, 'Cuba', 'CU', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1057, 'Cyprus', 'CY', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1058, 'Czech Republic', 'CZ', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1059, 'Denmark', 'DK', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1060, 'Djibouti', 'DJ', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1061, 'Dominica', 'DM', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1062, 'Dominican Republic', 'DO', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1063, 'Timor-Leste', 'TL', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1064, 'Ecuador', 'EC', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1065, 'Egypt', 'EG', NULL, NULL, NULL, NULL, 3, 0);
INSERT INTO `r_countries` VALUES(1066, 'El Salvador', 'SV', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1067, 'Equatorial Guinea', 'GQ', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1068, 'Eritrea', 'ER', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1069, 'Estonia', 'EE', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1070, 'Ethiopia', 'ET', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1072, 'Falkland Islands (Malvinas)', 'FK', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1073, 'Faroe Islands', 'FO', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1074, 'Fiji', 'FJ', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1075, 'Finland', 'FI', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1076, 'France', 'FR', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1077, 'French Guiana', 'GF', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1078, 'French Polynesia', 'PF', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1079, 'French Southern Territories', 'TF', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1080, 'Gabon', 'GA', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1081, 'Georgia', 'GE', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1082, 'Germany', 'DE', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1083, 'Ghana', 'GH', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1084, 'Gibraltar', 'GI', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1085, 'Greece', 'GR', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1086, 'Greenland', 'GL', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1087, 'Grenada', 'GD', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1088, 'Guadeloupe', 'GP', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1089, 'Guam', 'GU', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1090, 'Guatemala', 'GT', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1091, 'Guinea', 'GN', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1092, 'Guinea-Bissau', 'GW', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1093, 'Guyana', 'GY', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1094, 'Haiti', 'HT', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1095, 'Heard Island and McDonald Islands', 'HM', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1096, 'Holy See (Vatican City State)', 'VA', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1097, 'Honduras', 'HN', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1098, 'Hong Kong', 'HK', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1099, 'Hungary', 'HU', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1100, 'Iceland', 'IS', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1101, 'India', 'IN', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1102, 'Indonesia', 'ID', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1103, 'Iran, Islamic Republic of', 'IR', NULL, NULL, NULL, NULL, 3, 0);
INSERT INTO `r_countries` VALUES(1104, 'Iraq', 'IQ', NULL, NULL, NULL, NULL, 3, 0);
INSERT INTO `r_countries` VALUES(1105, 'Ireland', 'IE', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1106, 'Israel', 'IL', NULL, NULL, NULL, NULL, 3, 0);
INSERT INTO `r_countries` VALUES(1107, 'Italy', 'IT', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1108, 'Jamaica', 'JM', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1109, 'Japan', 'JP', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1110, 'Jordan', 'JO', NULL, NULL, NULL, NULL, 3, 0);
INSERT INTO `r_countries` VALUES(1111, 'Kazakhstan', 'KZ', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1112, 'Kenya', 'KE', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1113, 'Kiribati', 'KI', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1114, 'Korea, Democratic People\'s Republic of', 'KP', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1115, 'Korea, Republic of', 'KR', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1116, 'Kuwait', 'KW', NULL, NULL, NULL, NULL, 3, 0);
INSERT INTO `r_countries` VALUES(1117, 'Kyrgyzstan', 'KG', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1118, 'Lao People\'s Democratic Republic', 'LA', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1119, 'Latvia', 'LV', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1120, 'Lebanon', 'LB', NULL, NULL, NULL, NULL, 3, 0);
INSERT INTO `r_countries` VALUES(1121, 'Lesotho', 'LS', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1122, 'Liberia', 'LR', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1123, 'Libyan Arab Jamahiriya', 'LY', NULL, NULL, NULL, NULL, 3, 0);
INSERT INTO `r_countries` VALUES(1124, 'Liechtenstein', 'LI', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1125, 'Lithuania', 'LT', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1126, 'Luxembourg', 'LU', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1127, 'Macao', 'MO', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1128, 'Macedonia, Republic of', 'MK', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1129, 'Madagascar', 'MG', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1130, 'Malawi', 'MW', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1131, 'Malaysia', 'MY', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1132, 'Maldives', 'MV', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1133, 'Mali', 'ML', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1134, 'Malta', 'MT', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1135, 'Marshall Islands', 'MH', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1136, 'Martinique', 'MQ', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1137, 'Mauritania', 'MR', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1138, 'Mauritius', 'MU', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1139, 'Mayotte', 'YT', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1140, 'Mexico', 'MX', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1141, 'Micronesia, Federated States of', 'FM', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1142, 'Moldova', 'MD', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1143, 'Monaco', 'MC', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1144, 'Mongolia', 'MN', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1145, 'Montserrat', 'MS', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1146, 'Morocco', 'MA', NULL, NULL, NULL, NULL, 3, 0);
INSERT INTO `r_countries` VALUES(1147, 'Mozambique', 'MZ', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1148, 'Namibia', 'NA', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1149, 'Nauru', 'NR', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1150, 'Nepal', 'NP', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1151, 'Netherlands Antilles', 'AN', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1152, 'Netherlands', 'NL', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1153, 'New Caledonia', 'NC', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1154, 'New Zealand', 'NZ', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1155, 'Nicaragua', 'NI', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1156, 'Niger', 'NE', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1157, 'Nigeria', 'NG', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1158, 'Niue', 'NU', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1159, 'Norfolk Island', 'NF', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1160, 'Northern Mariana Islands', 'MP', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1161, 'Norway', 'NO', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1162, 'Oman', 'OM', NULL, NULL, NULL, NULL, 3, 0);
INSERT INTO `r_countries` VALUES(1163, 'Pakistan', 'PK', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1164, 'Palau', 'PW', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1165, 'Palestinian Territory, Occupied', 'PS', NULL, NULL, NULL, NULL, 3, 0);
INSERT INTO `r_countries` VALUES(1166, 'Panama', 'PA', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1167, 'Papua New Guinea', 'PG', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1168, 'Paraguay', 'PY', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1169, 'Peru', 'PE', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1170, 'Philippines', 'PH', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1171, 'Pitcairn', 'PN', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1172, 'Poland', 'PL', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1173, 'Portugal', 'PT', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1174, 'Puerto Rico', 'PR', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1175, 'Qatar', 'QA', NULL, NULL, NULL, NULL, 3, 0);
INSERT INTO `r_countries` VALUES(1176, 'Romania', 'RO', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1177, 'Russian Federation', 'RU', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1178, 'Rwanda', 'RW', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1179, 'Reunion', 'RE', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1180, 'Saint Helena', 'SH', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1181, 'Saint Kitts and Nevis', 'KN', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1182, 'Saint Lucia', 'LC', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1183, 'Saint Pierre and Miquelon', 'PM', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1184, 'Saint Vincent and the Grenadines', 'VC', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1185, 'Samoa', 'WS', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1186, 'San Marino', 'SM', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1187, 'Saudi Arabia', 'SA', NULL, NULL, NULL, NULL, 3, 0);
INSERT INTO `r_countries` VALUES(1188, 'Senegal', 'SN', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1189, 'Seychelles', 'SC', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1190, 'Sierra Leone', 'SL', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1191, 'Singapore', 'SG', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1192, 'Slovakia', 'SK', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1193, 'Slovenia', 'SI', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1194, 'Solomon Islands', 'SB', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1195, 'Somalia', 'SO', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1196, 'South Africa', 'ZA', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1197, 'South Georgia and the South Sandwich Islands', 'GS', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1198, 'Spain', 'ES', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1199, 'Sri Lanka', 'LK', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1200, 'Sudan', 'SD', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1201, 'Suriname', 'SR', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1202, 'Svalbard and Jan Mayen', 'SJ', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1203, 'Swaziland', 'SZ', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1204, 'Sweden', 'SE', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1205, 'Switzerland', 'CH', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1206, 'Syrian Arab Republic', 'SY', NULL, NULL, NULL, NULL, 3, 0);
INSERT INTO `r_countries` VALUES(1207, 'Sao Tome and Principe', 'ST', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1208, 'Taiwan', 'TW', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1209, 'Tajikistan', 'TJ', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1210, 'Tanzania, United Republic of', 'TZ', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1211, 'Thailand', 'TH', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1212, 'Bahamas', 'BS', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1213, 'Gambia', 'GM', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1214, 'Togo', 'TG', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1215, 'Tokelau', 'TK', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1216, 'Tonga', 'TO', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1217, 'Trinidad and Tobago', 'TT', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1218, 'Tunisia', 'TN', NULL, NULL, NULL, NULL, 3, 0);
INSERT INTO `r_countries` VALUES(1219, 'Turkey', 'TR', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1220, 'Turkmenistan', 'TM', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1221, 'Turks and Caicos Islands', 'TC', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1222, 'Tuvalu', 'TV', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1223, 'Uganda', 'UG', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1224, 'Ukraine', 'UA', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1225, 'United Arab Emirates', 'AE', NULL, NULL, NULL, NULL, 3, 0);
INSERT INTO `r_countries` VALUES(1226, 'United Kingdom', 'GB', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1227, 'United States Minor Outlying Islands', 'UM', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1228, 'United States', 'US', NULL, NULL, NULL, NULL, 2, 1);
INSERT INTO `r_countries` VALUES(1229, 'Uruguay', 'UY', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1230, 'Uzbekistan', 'UZ', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1231, 'Vanuatu', 'VU', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1232, 'Venezuela', 'VE', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1233, 'Viet Nam', 'VN', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1234, 'Virgin Islands, U.S.', 'VI', NULL, NULL, NULL, NULL, 2, 0);
INSERT INTO `r_countries` VALUES(1235, 'Wallis and Futuna', 'WF', NULL, NULL, NULL, NULL, 4, 0);
INSERT INTO `r_countries` VALUES(1236, 'Western Sahara', 'EH', NULL, NULL, NULL, NULL, 3, 0);
INSERT INTO `r_countries` VALUES(1237, 'Yemen', 'YE', NULL, NULL, NULL, NULL, 3, 0);
INSERT INTO `r_countries` VALUES(1238, 'Serbia and Montenegro', 'CS', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1239, 'Zambia', 'ZM', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1240, 'Zimbabwe', 'ZW', NULL, NULL, NULL, NULL, 5, 0);
INSERT INTO `r_countries` VALUES(1241, 'Ã…land Islands', 'AX', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1242, 'Serbia', 'RS', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1243, 'Montenegro', 'ME', NULL, NULL, NULL, NULL, 1, 0);
INSERT INTO `r_countries` VALUES(1244, 'Jersey', 'JE', NULL, NULL, NULL, NULL, 99, 0);
INSERT INTO `r_countries` VALUES(1245, 'Guernsey', 'GG', NULL, NULL, NULL, NULL, 99, 0);
INSERT INTO `r_countries` VALUES(1246, 'Isle of Man', 'IM', NULL, NULL, NULL, NULL, 99, 0);

-- --------------------------------------------------------

--
-- Table structure for table `r_criteria`
--

CREATE TABLE `r_criteria` (
  `id` bigint(20) NOT NULL COMMENT 'criterion record id',
  `name` text DEFAULT NULL COMMENT 'name of criterion',
  `text` text DEFAULT NULL COMMENT 'text of the criterion',
  `detail` longtext DEFAULT NULL COMMENT 'additional detail about the criterion',
  `points` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'how many points for this criterion',
  `auto` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'is this criterion calculated automatically?',
  `displayOrder` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'where to display this criterion in the order',
  `ctty` bigint(20) NOT NULL DEFAULT 0 COMMENT 'community or region record id (zero for all)',
  `modified` int(11) NOT NULL DEFAULT 0 COMMENT 'date/time last modified',
  `created` int(11) NOT NULL DEFAULT 0 COMMENT 'date/time created'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Criteria for funding proposals' ROW_FORMAT=DYNAMIC;

--
-- Dumping data for table `r_criteria`
--

INSERT INTO `r_criteria` VALUES(1, 'suitable', 'How well does the project support our Common Good community investment priorities?', NULL, 15.00, 0, 0, 0, 1541171548, 1541171548);
INSERT INTO `r_criteria` VALUES(2, 'systemic', 'How well does the project promote systemic change?', NULL, 15.00, 0, 0, 0, 1541171548, 1541171548);
INSERT INTO `r_criteria` VALUES(3, 'doable', 'Overall, how clearly doable is the project?', NULL, 15.00, 0, 0, 0, 1541171548, 1541171548);
INSERT INTO `r_criteria` VALUES(4, 'mgmt', 'How competent is the project team to manage the project and funds effectively?', NULL, 10.00, 0, 0, 0, 1541171548, 1541171548);
INSERT INTO `r_criteria` VALUES(5, 'cope', 'How able is the project team to implement the project with less funding than requested?', NULL, 5.00, 0, 0, 0, 1541171548, 1541171548);
INSERT INTO `r_criteria` VALUES(6, 'eval', 'How useful is the project\'s evaluation plan?', NULL, 5.00, 0, 0, 0, 1541171548, 1541171548);
INSERT INTO `r_criteria` VALUES(7, 'recovery', 'How quickly, surely, and voluminously will the project bring funds back into our Common Good Community Dollar Pool?', NULL, 10.00, 0, 0, 0, 1541171548, 1541171548);
INSERT INTO `r_criteria` VALUES(8, 'goodAmt', 'How close is the request to the ideal amount (%idealAmt)?', NULL, 5.00, 1, 0, 0, 1541171548, 1541171548);
INSERT INTO `r_criteria` VALUES(9, 'budgetPct', 'What fraction of the total project budget is this funding request? (50% is ideal)', NULL, 5.00, 1, 0, 0, 1541171548, 1541171548);
INSERT INTO `r_criteria` VALUES(10, 'committedPct', 'How much of the total project budget has been raised/committed so far? (half is ideal)?', NULL, 5.00, 1, 0, 0, 1541171548, 1541171548);
INSERT INTO `r_criteria` VALUES(11, 'beginSoon', 'How soon does the project begin? (soon after funding is best)', NULL, 2.50, 1, 0, 0, 1541171548, 1541171548);
INSERT INTO `r_criteria` VALUES(12, 'endSoon', 'How soon does the project end? (soon after funding is best)', NULL, 2.50, 1, 0, 0, 1541171548, 1541171548);
INSERT INTO `r_criteria` VALUES(13, 'local', 'How local is the project?', NULL, 2.50, 1, 0, 0, 1541171548, 1541171548);
INSERT INTO `r_criteria` VALUES(14, 'sponsor', 'Common Good member sponsorship of the project.', NULL, 2.50, 1, 0, 0, 1541171548, 1541171548);

-- --------------------------------------------------------

--
-- Table structure for table `r_do`
--

CREATE TABLE `r_do` (
  `doid` int(11) NOT NULL COMMENT 'record id',
  `expires` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime expiration',
  `data` longtext DEFAULT NULL COMMENT 'serialized array of parameters',
  `uid` bigint(20) NOT NULL DEFAULT 0 COMMENT 'related account record ID',
  `box` bigint(20) DEFAULT NULL COMMENT 'machine id',
  `created` int(11) DEFAULT 0 COMMENT 'Unixtime record was created',
  `completed` int(11) DEFAULT 0 COMMENT 'Unixtime action was completed',
  `action` longtext DEFAULT NULL COMMENT 'serialized array describing action to take',
  `code` varchar(255) DEFAULT NULL COMMENT 'security code',
  `before` int(11) DEFAULT 0 COMMENT 'Unixtime expiration'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Clickable actions with no signin' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `r_events`
--

CREATE TABLE `r_events` (
  `id` int(11) NOT NULL COMMENT 'record id',
  `ctty` bigint(20) DEFAULT NULL COMMENT 'what community the event is in',
  `type` char(1) DEFAULT NULL COMMENT 'event type (I=in person, V=vote, G=grading, P=RFP)',
  `event` varchar(255) DEFAULT NULL COMMENT 'name of event',
  `details` longtext DEFAULT NULL COMMENT 'event details',
  `start` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime event begins',
  `end` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime event ends'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Events in each community’s democratic process' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `r_honors`
--

CREATE TABLE `r_honors` (
  `id` int(11) NOT NULL COMMENT 'record id',
  `uid` bigint(20) DEFAULT NULL COMMENT 'uid of account making the gift to Common Good',
  `honor` varchar(10) DEFAULT NULL COMMENT 'what type of honor',
  `honored` longtext DEFAULT NULL COMMENT 'who is honored',
  `created` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime of first associated gift'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='gifts in honor or memory' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `r_industries`
--

CREATE TABLE `r_industries` (
  `iid` int(11) NOT NULL,
  `industry` varchar(255) NOT NULL,
  `parent` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC;

--
-- Dumping data for table `r_industries`
--

INSERT INTO `r_industries` VALUES(1, 'Art / Culture / Science', NULL);
INSERT INTO `r_industries` VALUES(2, 'Food', NULL);
INSERT INTO `r_industries` VALUES(3, 'Business and Industry', NULL);
INSERT INTO `r_industries` VALUES(4, 'Building / Construction / Hardware', NULL);
INSERT INTO `r_industries` VALUES(5, 'Family, Community, and Society', NULL);
INSERT INTO `r_industries` VALUES(6, 'Education', NULL);
INSERT INTO `r_industries` VALUES(7, 'Money', NULL);
INSERT INTO `r_industries` VALUES(8, 'Information and Communications', NULL);
INSERT INTO `r_industries` VALUES(9, 'Sports and Recreation', NULL);
INSERT INTO `r_industries` VALUES(10, 'Energy and Fuel', NULL);
INSERT INTO `r_industries` VALUES(11, 'Health', NULL);
INSERT INTO `r_industries` VALUES(12, 'Transportation', NULL);
INSERT INTO `r_industries` VALUES(13, 'Place', NULL);
INSERT INTO `r_industries` VALUES(14, 'Retail', NULL);
INSERT INTO `r_industries` VALUES(15, 'Other Services', NULL);
INSERT INTO `r_industries` VALUES(16, 'Visual Artists', 1);
INSERT INTO `r_industries` VALUES(17, 'Performance Artists', 1);
INSERT INTO `r_industries` VALUES(18, 'Museums and Galleries', 1);
INSERT INTO `r_industries` VALUES(19, 'Visual Art', 1);
INSERT INTO `r_industries` VALUES(20, 'Theaters and Performing Arts', 1);
INSERT INTO `r_industries` VALUES(21, 'Arts and Crafts Supplies', 1);
INSERT INTO `r_industries` VALUES(22, 'Farms', 2);
INSERT INTO `r_industries` VALUES(23, 'Grocers', 2);
INSERT INTO `r_industries` VALUES(24, 'Restaurants and Catering', 2);
INSERT INTO `r_industries` VALUES(25, 'Food Processing and Distribution', 2);
INSERT INTO `r_industries` VALUES(26, 'Engineering', 3);
INSERT INTO `r_industries` VALUES(27, 'Office Supplies', 3);
INSERT INTO `r_industries` VALUES(28, 'Computer Services', 3);
INSERT INTO `r_industries` VALUES(29, 'Manufacturing and Mining', 3);
INSERT INTO `r_industries` VALUES(30, 'Industrial Supplies and Materials', 3);
INSERT INTO `r_industries` VALUES(31, 'Business Services and Associations', 3);
INSERT INTO `r_industries` VALUES(32, 'Wholesale', 3);
INSERT INTO `r_industries` VALUES(33, 'Hardware and Building Supplies', 4);
INSERT INTO `r_industries` VALUES(34, 'Plumbing / Heating / Cooling', 4);
INSERT INTO `r_industries` VALUES(35, 'Electrical and Lighting', 4);
INSERT INTO `r_industries` VALUES(36, 'Contractors', 4);
INSERT INTO `r_industries` VALUES(37, 'Construction Equipment', 4);
INSERT INTO `r_industries` VALUES(38, 'Family and Human Services', 5);
INSERT INTO `r_industries` VALUES(39, 'Community Organizations', 5);
INSERT INTO `r_industries` VALUES(40, 'Government', 5);
INSERT INTO `r_industries` VALUES(41, 'Law', 5);
INSERT INTO `r_industries` VALUES(42, 'Educational Adventures', 6);
INSERT INTO `r_industries` VALUES(43, 'Schools and Colleges', 6);
INSERT INTO `r_industries` VALUES(44, 'Credit Unions and Banks', 7);
INSERT INTO `r_industries` VALUES(45, 'Insurance', 7);
INSERT INTO `r_industries` VALUES(46, 'Financial Services', 7);
INSERT INTO `r_industries` VALUES(47, 'Media', 8);
INSERT INTO `r_industries` VALUES(48, 'Internet', 8);
INSERT INTO `r_industries` VALUES(49, 'Libraries', 8);
INSERT INTO `r_industries` VALUES(50, 'Electronics', 8);
INSERT INTO `r_industries` VALUES(51, 'Hobbies', 9);
INSERT INTO `r_industries` VALUES(52, 'Sports Equipment', 9);
INSERT INTO `r_industries` VALUES(53, 'Liquor, Tobacco, and Other Recreational Drugs', 9);
INSERT INTO `r_industries` VALUES(54, 'Solar, Wind, and Alternative Energy', 10);
INSERT INTO `r_industries` VALUES(55, 'Propane and Oil', 10);
INSERT INTO `r_industries` VALUES(56, 'Gasoline', 10);
INSERT INTO `r_industries` VALUES(57, 'Gyms', 11);
INSERT INTO `r_industries` VALUES(58, 'Therapists', 11);
INSERT INTO `r_industries` VALUES(59, 'Hospitals / Clinics / Doctors / Midwives', 11);
INSERT INTO `r_industries` VALUES(60, 'Medical Supplies and Drugstores', 11);
INSERT INTO `r_industries` VALUES(61, 'Public Transit, Carpooling, and Buses', 12);
INSERT INTO `r_industries` VALUES(62, 'Bicycles', 12);
INSERT INTO `r_industries` VALUES(63, 'Cars / Automobiles', 12);
INSERT INTO `r_industries` VALUES(64, 'Automotive Services', 12);
INSERT INTO `r_industries` VALUES(65, 'Housing', 13);
INSERT INTO `r_industries` VALUES(66, 'Accommodations', 13);
INSERT INTO `r_industries` VALUES(67, 'Facilities', 13);
INSERT INTO `r_industries` VALUES(68, 'Real Estate', 13);
INSERT INTO `r_industries` VALUES(69, 'Architects', 13);
INSERT INTO `r_industries` VALUES(70, 'Department Stores', 14);
INSERT INTO `r_industries` VALUES(71, 'Bookstores', 14);
INSERT INTO `r_industries` VALUES(72, 'Furniture', 14);
INSERT INTO `r_industries` VALUES(73, 'Appliances', 14);
INSERT INTO `r_industries` VALUES(74, 'Clothing', 14);
INSERT INTO `r_industries` VALUES(75, 'Consumer Electronics', 14);
INSERT INTO `r_industries` VALUES(76, 'Toys and Games', 14);
INSERT INTO `r_industries` VALUES(77, 'Other Retail', 14);
INSERT INTO `r_industries` VALUES(78, 'Farm Equipment and Supplies', 22);

-- --------------------------------------------------------

--
-- Table structure for table `r_investments`
--

CREATE TABLE `r_investments` (
  `vestid` bigint(20) NOT NULL COMMENT 'record ID',
  `coid` bigint(20) DEFAULT NULL COMMENT 'member company record ID',
  `clubid` bigint(20) DEFAULT NULL COMMENT 'investment club record ID',
  `proposedBy` bigint(20) DEFAULT NULL COMMENT 'account record ID of proposer',
  `investment` longtext DEFAULT NULL COMMENT 'description of investment',
  `return` decimal(7,6) DEFAULT NULL COMMENT 'predicted or actual APR',
  `types` varchar(4) DEFAULT NULL COMMENT 'D=dividends I=interest T=tax-exempt interest',
  `terms` mediumtext DEFAULT NULL COMMENT 'investment terms',
  `assets` decimal(11,2) DEFAULT NULL COMMENT 'company assets, bond, or collateral',
  `offering` decimal(11,2) DEFAULT NULL COMMENT 'size of offering',
  `price` decimal(11,2) DEFAULT NULL COMMENT 'price per share',
  `character` longtext DEFAULT NULL COMMENT 'assessment of the integrity and determination of the owners',
  `strength` tinyint(4) DEFAULT NULL COMMENT 'company''s financial strength (0 to 100)',
  `web` tinyint(4) DEFAULT NULL COMMENT 'impression on the web (0 to 100)',
  `history` tinyint(4) DEFAULT NULL COMMENT 'past repayment success (0 to 100)',
  `soundness` tinyint(4) DEFAULT NULL COMMENT 'overall how sound is this investment (0 to 100)',
  `reserve` decimal(10,3) NOT NULL DEFAULT 0.000 COMMENT 'fraction to hold in reserve for possible loss'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='potential and actual investments' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `r_invites`
--

CREATE TABLE `r_invites` (
  `id` bigint(20) NOT NULL COMMENT 'record id',
  `code` varchar(64) DEFAULT NULL COMMENT 'secret invitation code',
  `email` varchar(255) DEFAULT NULL COMMENT 'email of invitee',
  `inviter` bigint(20) NOT NULL DEFAULT 0 COMMENT 'uid of inviting member',
  `invitee` bigint(20) NOT NULL DEFAULT 0 COMMENT 'uid of invited new member',
  `invited` int(11) NOT NULL DEFAULT 0 COMMENT 'date of invitation',
  `subject` varchar(255) DEFAULT NULL COMMENT 'email subject',
  `message` longtext DEFAULT NULL COMMENT 'email message body',
  `zip` varchar(10) NOT NULL DEFAULT 'NULL' COMMENT 'alleged postal code of recipient',
  `nonudge` int(11) DEFAULT NULL COMMENT 'date/time this invitee "unsubscribed"'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Who invited whom' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `r_ips`
--

CREATE TABLE `r_ips` (
  `ip` varchar(39) NOT NULL COMMENT 'ip address',
  `uid` bigint(20) DEFAULT NULL COMMENT 'account record ID',
  `device` varchar(255) DEFAULT NULL COMMENT 'device code'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='IP addresses of approved accounts' ROW_FORMAT=DYNAMIC;

--
-- Dumping data for table `r_ips`
--

INSERT INTO `r_ips` VALUES('::1', 410044002, 'meXEyYSfdVYUHjXCp7kP');
INSERT INTO `r_ips` VALUES('127.0.0.1', 410061553, 'gxKug30M8Oa4SbFJZq2s');

-- --------------------------------------------------------

--
-- Table structure for table `r_near`
--

CREATE TABLE `r_near` (
  `uid1` bigint(20) NOT NULL COMMENT 'account record ID of one account',
  `uid2` bigint(20) NOT NULL COMMENT 'account record ID of other account',
  `weight` mediumint(9) DEFAULT NULL COMMENT 'number of connections'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='How members are connected' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `r_notices`
--

CREATE TABLE `r_notices` (
  `msgid` int(11) NOT NULL COMMENT 'notice record id',
  `uid` bigint(20) DEFAULT NULL COMMENT 'account record ID of member notified',
  `created` int(11) NOT NULL DEFAULT 0 COMMENT 'date of notice',
  `type` varchar(255) DEFAULT NULL COMMENT 'type of notice',
  `sent` int(11) NOT NULL DEFAULT 0 COMMENT 'date sent (0 if not sent yet)',
  `message` longtext DEFAULT NULL COMMENT 'the notice text'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Message digest buffer' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `r_options`
--

CREATE TABLE `r_options` (
  `id` bigint(20) NOT NULL COMMENT 'option record id',
  `question` bigint(20) NOT NULL DEFAULT 0 COMMENT 'question for which this is an option',
  `text` text DEFAULT NULL COMMENT 'text of the option',
  `detail` longtext DEFAULT NULL COMMENT 'additional detail about the option',
  `displayOrder` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'where to display this option in the order',
  `minimum` bigint(20) NOT NULL DEFAULT 0 COMMENT 'the least (money) to budget for this option',
  `maximum` bigint(20) NOT NULL DEFAULT 0 COMMENT 'the most (money) to budget for this option',
  `mandatory` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'is the minimum required?',
  `averageGrade` decimal(5,3) NOT NULL DEFAULT 0.000 COMMENT 'average grade (in a penny vote, the fraction of all votes)',
  `averageMax` decimal(5,3) NOT NULL DEFAULT 0.000 COMMENT 'average maximum grade (for range votes)',
  `vetoes` int(11) NOT NULL DEFAULT 0 COMMENT 'number of vetoes for this option',
  `modified` int(11) NOT NULL DEFAULT 0 COMMENT 'date/time last modified',
  `created` int(11) NOT NULL DEFAULT 0 COMMENT 'date/time created'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Options for a question to be voted on' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `r_pairs`
--

CREATE TABLE `r_pairs` (
  `id` bigint(20) NOT NULL COMMENT 'pairs record id',
  `option1` bigint(20) NOT NULL DEFAULT 0 COMMENT 'record id of one option',
  `option2` bigint(20) NOT NULL DEFAULT 0 COMMENT 'record id of the other option',
  `prefer1` int(11) NOT NULL DEFAULT 0 COMMENT 'how many voters prefer the option1',
  `prefer2` int(11) NOT NULL DEFAULT 0 COMMENT 'how many voters prefer the option2',
  `nopreference` int(11) NOT NULL DEFAULT 0 COMMENT 'how many voters had no preference between the two options',
  `raw` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'true if this record is calculated without counting proxies',
  `created` int(11) NOT NULL DEFAULT 0 COMMENT 'date/time created'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Counts of preferences of one option over another' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `r_proposals`
--

CREATE TABLE `r_proposals` (
  `id` bigint(20) NOT NULL COMMENT 'proposal record id',
  `event` bigint(20) DEFAULT NULL COMMENT 'what event this question is part of',
  `project` varchar(255) DEFAULT NULL COMMENT 'project title',
  `overview` longtext DEFAULT NULL COMMENT 'project overview',
  `categories` text DEFAULT NULL COMMENT 'funding categories (space-separated list)',
  `purpose` longtext DEFAULT NULL COMMENT 'why the project is needed',
  `systemic` longtext DEFAULT NULL COMMENT 'how the project promotes systemic change',
  `where` text DEFAULT NULL COMMENT 'zipcode where the project will take place',
  `when` int(11) DEFAULT NULL COMMENT 'project start date',
  `until` int(11) DEFAULT NULL COMMENT 'project end date',
  `how` longtext DEFAULT NULL COMMENT 'how the project will be accomplished',
  `amount` decimal(11,2) DEFAULT NULL COMMENT 'amount of funding requested',
  `type` tinyint(4) DEFAULT NULL COMMENT 'type of funding',
  `recovery` mediumint(9) NOT NULL DEFAULT 0 COMMENT 'how funds will return to the Community Fund',
  `budgetTotal` decimal(11,2) DEFAULT NULL COMMENT 'total expense budget',
  `budget` longtext DEFAULT NULL COMMENT 'detailed project budget',
  `committed` decimal(11,2) DEFAULT NULL COMMENT 'how much of the budget has already been raised',
  `contingency` longtext DEFAULT NULL COMMENT 'how the project organizers will cope with less funding than requested',
  `qualifications` longtext DEFAULT NULL COMMENT 'qualifications of project staff',
  `evaluation` longtext DEFAULT NULL COMMENT 'how the success of the project will be evaluated',
  `name` varchar(255) DEFAULT NULL COMMENT 'individual or organization making the proposal',
  `contact` varchar(255) DEFAULT NULL COMMENT 'contact person (or "self")',
  `phone` varchar(255) DEFAULT NULL COMMENT 'contact phone',
  `email` varchar(255) DEFAULT NULL COMMENT 'contact email',
  `sponsor` text DEFAULT NULL COMMENT 'member(s) sponsoring the project proposal',
  `ctty` bigint(20) DEFAULT NULL COMMENT 'community or region record id'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='rCredits funding proposals' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `r_proxies`
--

CREATE TABLE `r_proxies` (
  `id` int(11) NOT NULL COMMENT 'record id',
  `person` bigint(20) DEFAULT NULL COMMENT 'account record id',
  `proxy` bigint(20) DEFAULT NULL COMMENT 'account record id of proxy',
  `priority` tinyint(4) DEFAULT NULL COMMENT 'precedence of this proxy (1=top priority)'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Who represents whom' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `r_questions`
--

CREATE TABLE `r_questions` (
  `id` bigint(20) NOT NULL COMMENT 'question record id',
  `event` bigint(20) DEFAULT NULL COMMENT 'what event this question is part of',
  `repeats` bigint(20) NOT NULL DEFAULT 0 COMMENT 'pointer to question that this is a revote on (0=none)',
  `repeatedBy` bigint(20) NOT NULL DEFAULT 0 COMMENT 'pointer to question that is a revote of this one (0=none))',
  `text` longtext DEFAULT NULL COMMENT 'text of the question',
  `detail` longtext DEFAULT NULL COMMENT 'additional detail about the question',
  `linkDiscussion` text DEFAULT NULL COMMENT 'link to online discussion of the issue',
  `type` char(1) NOT NULL DEFAULT 'M' COMMENT 'vote type M=multiple choice, B=budget (penny vote), R=range, E=essay',
  `units` varchar(255) DEFAULT NULL COMMENT 'budget units (defaults to money, measured in the community''s national currency)',
  `budget` decimal(11,0) NOT NULL DEFAULT 0 COMMENT 'how much (money) is to be budgeted',
  `minVeto` decimal(5,3) DEFAULT NULL COMMENT 'minimum veto fraction of vote, to force reconsideration',
  `optOrder` char(1) NOT NULL DEFAULT 'S' COMMENT 'option order',
  `voteCount` int(11) NOT NULL DEFAULT 0 COMMENT 'total number of votes',
  `result` longtext DEFAULT NULL COMMENT 'results of the vote or grading',
  `created` int(11) NOT NULL DEFAULT 0 COMMENT 'date/time created'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Questions to be voted on' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `r_ratings`
--

CREATE TABLE `r_ratings` (
  `ratingid` bigint(20) NOT NULL COMMENT 'record ID',
  `vestid` int(11) DEFAULT NULL COMMENT 'investment record ID',
  `uid` bigint(20) DEFAULT NULL COMMENT 'member record ID',
  `good` tinyint(4) DEFAULT NULL COMMENT 'how well this investment serves the common good (0-100)',
  `recurs` decimal(11,2) DEFAULT NULL COMMENT 'how much the member will spend here monthly',
  `comment` longtext DEFAULT NULL COMMENT 'description of investment',
  `patronage` decimal(11,2) DEFAULT NULL COMMENT 'how much the member will spend here monthly'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='how a member rates an investment' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `r_regions`
--

CREATE TABLE `r_regions` (
  `region` char(3) NOT NULL DEFAULT '' COMMENT 'region id',
  `fullName` varchar(255) DEFAULT NULL,
  `st` char(2) DEFAULT NULL COMMENT 'state or province abbreviation',
  `zips` text DEFAULT NULL COMMENT 'zip regex for this geographic region',
  `postalAddr` varchar(255) DEFAULT NULL,
  `federalId` varchar(9) NOT NULL,
  `hasServer` tinyint(1) NOT NULL DEFAULT 0
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC;

--
-- Dumping data for table `r_regions`
--

INSERT INTO `r_regions` VALUES('AZA', NULL, 'AZ', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('CAA', NULL, 'CA', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('CLN', NULL, 'NC', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('CLS', NULL, 'SC', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('FLA', NULL, 'FL', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('GAA', NULL, 'GA', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('ILA', NULL, 'IL', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('INA', NULL, 'IN', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('MDA', NULL, 'MD', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('MIA', NULL, 'MI', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('MIW', 'Washtenaw County', 'MI', '^492|^48(103|104|105|107|108|109|113|115|130|158|175|176|190|191|197|198)', 'c/o A. Konner, 1005 Packard St., Ann Arbor, MI 48104', '', 0);
INSERT INTO `r_regions` VALUES('MOA', NULL, 'MO', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('MWE', NULL, 'NE', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('MWI', NULL, 'IA', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('MWK', NULL, 'KS', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('MWM', NULL, 'MN', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('MWN', NULL, 'ND', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('MWS', NULL, 'SD', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('NEB', 'Eastern Mass', 'MA', '^02|^017|^018|^019', NULL, '', 0);
INSERT INTO `r_regions` VALUES('NEC', NULL, 'CT', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('NEM', NULL, 'ME', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('NEN', NULL, 'NH', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('NER', NULL, 'RI', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('NEV', NULL, 'VT', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('NEW', 'Western Mass', 'MA', NULL, 'c/o Common Good Finance, PO Box 21, Ashfield, MA 01330', '461821792', 1);
INSERT INTO `r_regions` VALUES('NJA', NULL, 'NJ', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('NWO', NULL, 'OR', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('NWW', NULL, 'WA', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('NYA', NULL, 'NY', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('OHA', NULL, 'OH', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('PAA', NULL, 'PA', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('RMC', NULL, 'CO', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('RMI', NULL, 'ID', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('RMM', NULL, 'MT', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('RMN', NULL, 'NM', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('RMU', NULL, 'UT', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('RMV', NULL, 'NV', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('RMW', NULL, 'WY', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('TNA', NULL, 'TN', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('TXA', NULL, 'TX', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('UAG', NULL, 'GU', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('UAH', NULL, 'HI', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('UAK', NULL, 'AK', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('UAM', NULL, 'MP', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('UAP', NULL, 'PR', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('UAS', NULL, 'AS', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('UAV', NULL, 'VI', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('USB', NULL, 'AL', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('USC', NULL, 'DC', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('USD', NULL, 'DE', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('USK', NULL, 'KY', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('USL', NULL, 'LA', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('USM', NULL, 'MS', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('USO', NULL, 'OK', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('USR', NULL, 'AR', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('VRA', NULL, 'VA', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('VRW', NULL, 'WV', NULL, NULL, '', 0);
INSERT INTO `r_regions` VALUES('WIA', NULL, 'WI', NULL, NULL, '', 0);

-- --------------------------------------------------------

--
-- Table structure for table `r_shares`
--

CREATE TABLE `r_shares` (
  `shid` bigint(20) NOT NULL COMMENT 'record ID',
  `vestid` int(11) DEFAULT NULL COMMENT 'investment record ID',
  `shares` int(11) NOT NULL DEFAULT 0 COMMENT 'club shares in the investment bought or (if <0) sold',
  `pending` int(11) NOT NULL DEFAULT 0 COMMENT 'number of shares to buy or (if <0) sell ASAP',
  `when` int(11) DEFAULT NULL COMMENT 'Unixtime investment made',
  `sold` int(11) DEFAULT NULL COMMENT 'Unixtime investment totally sold'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='club stakes in investments' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `r_stakes`
--

CREATE TABLE `r_stakes` (
  `stakeid` bigint(20) NOT NULL COMMENT 'record ID',
  `uid` bigint(20) NOT NULL DEFAULT 0 COMMENT 'member record ID',
  `clubid` bigint(20) NOT NULL DEFAULT 0 COMMENT 'investment club record ID',
  `stake` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'member stake in the club',
  `request` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'request to change stake by this amount',
  `joined` int(11) NOT NULL DEFAULT 0 COMMENT 'when this member joined the club',
  `requestedOut` int(11) NOT NULL DEFAULT 0 COMMENT 'when this member last requested cash out'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='member stakes in an investment club' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `r_states`
--

CREATE TABLE `r_states` (
  `id` int(10) UNSIGNED NOT NULL COMMENT 'State / Province ID',
  `name` varchar(64) DEFAULT NULL COMMENT 'Name of State / Province',
  `abbreviation` varchar(4) DEFAULT NULL COMMENT '2-4 Character Abbreviation of State / Province',
  `country_id` int(10) UNSIGNED NOT NULL COMMENT 'ID of Country that State / Province belongs to'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC;

--
-- Dumping data for table `r_states`
--

INSERT INTO `r_states` VALUES
(2515, 'Cher', '18', 1076),
(1000, 'Alabama', 'AL', 1228),
(1001, 'Alaska', 'AK', 1228),
(1002, 'Arizona', 'AZ', 1228),
(1003, 'Arkansas', 'AR', 1228),
(1004, 'California', 'CA', 1228),
(1005, 'Colorado', 'CO', 1228),
(1006, 'Connecticut', 'CT', 1228),
(1007, 'Delaware', 'DE', 1228),
(1008, 'Florida', 'FL', 1228),
(1009, 'Georgia', 'GA', 1228),
(1010, 'Hawaii', 'HI', 1228),
(1011, 'Idaho', 'ID', 1228),
(1012, 'Illinois', 'IL', 1228),
(1013, 'Indiana', 'IN', 1228),
(1014, 'Iowa', 'IA', 1228),
(1015, 'Kansas', 'KS', 1228),
(1016, 'Kentucky', 'KY', 1228),
(1017, 'Louisiana', 'LA', 1228),
(1018, 'Maine', 'ME', 1228),
(1019, 'Maryland', 'MD', 1228),
(1020, 'Massachusetts', 'MA', 1228),
(1021, 'Michigan', 'MI', 1228),
(1022, 'Minnesota', 'MN', 1228),
(1023, 'Mississippi', 'MS', 1228),
(1024, 'Missouri', 'MO', 1228),
(1025, 'Montana', 'MT', 1228),
(1026, 'Nebraska', 'NE', 1228),
(1027, 'Nevada', 'NV', 1228),
(1028, 'New Hampshire', 'NH', 1228),
(1029, 'New Jersey', 'NJ', 1228),
(1030, 'New Mexico', 'NM', 1228),
(1031, 'New York', 'NY', 1228),
(1032, 'North Carolina', 'NC', 1228),
(1033, 'North Dakota', 'ND', 1228),
(1034, 'Ohio', 'OH', 1228),
(1035, 'Oklahoma', 'OK', 1228),
(1036, 'Oregon', 'OR', 1228),
(1037, 'Pennsylvania', 'PA', 1228),
(1038, 'Rhode Island', 'RI', 1228),
(1039, 'South Carolina', 'SC', 1228),
(1040, 'South Dakota', 'SD', 1228),
(1041, 'Tennessee', 'TN', 1228),
(1042, 'Texas', 'TX', 1228),
(1043, 'Utah', 'UT', 1228),
(1044, 'Vermont', 'VT', 1228),
(1045, 'Virginia', 'VA', 1228),
(1046, 'Washington', 'WA', 1228),
(1047, 'West Virginia', 'WV', 1228),
(1048, 'Wisconsin', 'WI', 1228),
(1049, 'Wyoming', 'WY', 1228),
(1050, 'District of Columbia', 'DC', 1228),
(1052, 'American Samoa', 'AS', 1228),
(1053, 'Guam', 'GU', 1228),
(1055, 'Northern Mariana Islands', 'MP', 1228),
(1056, 'Puerto Rico', 'PR', 1228),
(1057, 'Virgin Islands', 'VI', 1228),
(1058, 'United States Minor Outlying Islands', 'UM', 1228),
(1059, 'Armed Forces Europe', 'AE', 1228),
(1060, 'Armed Forces Americas', 'AA', 1228),
(1061, 'Armed Forces Pacific', 'AP', 1228);

-- --------------------------------------------------------

--
-- Table structure for table `r_stats`
--

CREATE TABLE `r_stats` (
  `id` bigint(20) NOT NULL COMMENT 'statistics record id',
  `created` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime record was created',
  `ctty` bigint(20) DEFAULT NULL COMMENT 'community or region record id',
  `pAccts` mediumint(9) NOT NULL DEFAULT 0 COMMENT 'number of personal accounts',
  `bAccts` mediumint(9) NOT NULL DEFAULT 0 COMMENT 'number of company accounts',
  `newbs` mediumint(9) NOT NULL DEFAULT 0 COMMENT 'number of not-yet-active accounts',
  `aAccts` mediumint(9) NOT NULL DEFAULT 0 COMMENT 'number of active personal accounts',
  `conx` decimal(10,3) NOT NULL DEFAULT 0.000 COMMENT 'number of connections per personal account',
  `conxLocal` decimal(10,3) NOT NULL DEFAULT 0.000 COMMENT 'number of local connections per personal account',
  `balsPos` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'amount of positive balances',
  `balsNeg` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'amount of negative balances',
  `balsPosCount` mediumint(9) NOT NULL DEFAULT 0 COMMENT 'number of positive balances',
  `balsNegCount` mediumint(9) NOT NULL DEFAULT 0 COMMENT 'number of negative balances',
  `topN` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'top N or N% of balances, whichever is greater',
  `botN` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'bottom N or N% of balances, whichever is less',
  `floors` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'credit lines',
  `p2b` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'customer purchase volume',
  `b2b` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'b2b purchase volume',
  `b2p` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'payroll volume',
  `p2p` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'person-to-person purchase volume',
  `p2bCount` mediumint(9) NOT NULL DEFAULT 0 COMMENT 'customer purchase volume',
  `b2bCount` mediumint(9) NOT NULL DEFAULT 0 COMMENT 'b2b purchase volume',
  `b2pCount` mediumint(9) NOT NULL DEFAULT 0 COMMENT 'payroll volume',
  `p2pCount` mediumint(9) NOT NULL DEFAULT 0 COMMENT 'person-to-person purchase volume',
  `cashs` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'amount of exchanges for cash',
  `cashsCount` mediumint(9) NOT NULL DEFAULT 0 COMMENT 'number of exchanges for cash',
  `cgIn` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'amount of Common Good Credits coming into this community',
  `cgOut` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'amount of Common Good Credits leaving this community',
  `cgInCount` mediumint(9) NOT NULL DEFAULT 0 COMMENT 'number of transfers into this community',
  `cgOutCount` mediumint(9) NOT NULL DEFAULT 0 COMMENT 'number of transfers out of this community',
  `usdIn` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'amount of US Dollars brought into the system',
  `usdOut` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'amount of US Dollars taken out of the system',
  `usdInCount` mediumint(9) NOT NULL DEFAULT 0 COMMENT 'number of incoming bank transfers',
  `usdOutCount` mediumint(9) NOT NULL DEFAULT 0 COMMENT 'number of outgoing bank transfers',
  `payees` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'median number of payees per active account in the recent past',
  `basket` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'median (positive) amount per transaction in the recent past',
  `patronage` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'current intended recurring donations per month',
  `roundups` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'average roundups per month in the recent past',
  `crumbs` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'average crumbs per month in the recent past',
  `invites` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'total invitations to date'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Operating statistics for communities and overall' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `r_tous`
--

CREATE TABLE `r_tous` (
  `id` int(11) NOT NULL COMMENT 'vote record id',
  `uid` bigint(20) DEFAULT NULL COMMENT 'account record ID',
  `time` int(11) NOT NULL DEFAULT 0 COMMENT 'date/time of message',
  `message` blob DEFAULT NULL COMMENT 'the message'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Secure messages sent from member to us.' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `r_transit`
--

CREATE TABLE `r_transit` (
  `id` int(11) NOT NULL,
  `location` varchar(255) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC;

--
-- Dumping data for table `r_transit`
--

INSERT INTO `r_transit` VALUES(93, '12');
INSERT INTO `r_transit` VALUES(59, 'AK');
INSERT INTO `r_transit` VALUES(61, 'AL');
INSERT INTO `r_transit` VALUES(29, 'Albany, NY');
INSERT INTO `r_transit` VALUES(81, 'AR');
INSERT INTO `r_transit` VALUES(59, 'AS');
INSERT INTO `r_transit` VALUES(91, 'AZ');
INSERT INTO `r_transit` VALUES(7, 'Baltimore, MD');
INSERT INTO `r_transit` VALUES(5, 'Boston, MA');
INSERT INTO `r_transit` VALUES(10, 'Buffalo, NY');
INSERT INTO `r_transit` VALUES(90, 'CA');
INSERT INTO `r_transit` VALUES(47, 'Cedar Rapids, IA');
INSERT INTO `r_transit` VALUES(2, 'Chicago, IL');
INSERT INTO `r_transit` VALUES(13, 'Cincinnati, OH');
INSERT INTO `r_transit` VALUES(6, 'Cleveland, OH');
INSERT INTO `r_transit` VALUES(82, 'CO');
INSERT INTO `r_transit` VALUES(25, 'Columbus, OH');
INSERT INTO `r_transit` VALUES(51, 'CT');
INSERT INTO `r_transit` VALUES(32, 'Dallas, TX');
INSERT INTO `r_transit` VALUES(62, 'DE');
INSERT INTO `r_transit` VALUES(23, 'Denver, CO');
INSERT INTO `r_transit` VALUES(33, 'Des Moines, IA');
INSERT INTO `r_transit` VALUES(9, 'Detroit, MI');
INSERT INTO `r_transit` VALUES(45, 'Dubuque, IA');
INSERT INTO `r_transit` VALUES(63, 'FL');
INSERT INTO `r_transit` VALUES(37, 'Fort Worth, TX');
INSERT INTO `r_transit` VALUES(64, 'GA');
INSERT INTO `r_transit` VALUES(46, 'Galveston, TX');
INSERT INTO `r_transit` VALUES(59, 'GU');
INSERT INTO `r_transit` VALUES(59, 'HI');
INSERT INTO `r_transit` VALUES(35, 'Houston, TX');
INSERT INTO `r_transit` VALUES(72, 'IA');
INSERT INTO `r_transit` VALUES(92, 'ID');
INSERT INTO `r_transit` VALUES(70, 'IL');
INSERT INTO `r_transit` VALUES(71, 'IN');
INSERT INTO `r_transit` VALUES(20, 'Indianapolis, IN');
INSERT INTO `r_transit` VALUES(18, 'Kansas City, MO');
INSERT INTO `r_transit` VALUES(83, 'KS');
INSERT INTO `r_transit` VALUES(73, 'KY');
INSERT INTO `r_transit` VALUES(84, 'LA');
INSERT INTO `r_transit` VALUES(43, 'Lincoln, NE');
INSERT INTO `r_transit` VALUES(16, 'Los Angeles, CA');
INSERT INTO `r_transit` VALUES(21, 'Louisville, KY');
INSERT INTO `r_transit` VALUES(53, 'MA');
INSERT INTO `r_transit` VALUES(52, 'ME');
INSERT INTO `r_transit` VALUES(26, 'Memphis, TN');
INSERT INTO `r_transit` VALUES(74, 'MI');
INSERT INTO `r_transit` VALUES(12, 'Milwaukee, WI');
INSERT INTO `r_transit` VALUES(17, 'Minneapolis, MN');
INSERT INTO `r_transit` VALUES(75, 'MN');
INSERT INTO `r_transit` VALUES(80, 'MO');
INSERT INTO `r_transit` VALUES(85, 'MS');
INSERT INTO `r_transit` VALUES(49, 'Muskogee, OK');
INSERT INTO `r_transit` VALUES(65, 'MY');
INSERT INTO `r_transit` VALUES(66, 'NC');
INSERT INTO `r_transit` VALUES(77, 'ND');
INSERT INTO `r_transit` VALUES(76, 'NE');
INSERT INTO `r_transit` VALUES(14, 'New Orleans, LA');
INSERT INTO `r_transit` VALUES(1, 'New York, NY');
INSERT INTO `r_transit` VALUES(54, 'NH');
INSERT INTO `r_transit` VALUES(55, 'NJ');
INSERT INTO `r_transit` VALUES(95, 'NM');
INSERT INTO `r_transit` VALUES(94, 'NV');
INSERT INTO `r_transit` VALUES(50, 'NY');
INSERT INTO `r_transit` VALUES(56, 'OH');
INSERT INTO `r_transit` VALUES(86, 'OK');
INSERT INTO `r_transit` VALUES(39, 'Oklahoma City, OK');
INSERT INTO `r_transit` VALUES(27, 'Omaha, NE');
INSERT INTO `r_transit` VALUES(96, 'OR');
INSERT INTO `r_transit` VALUES(60, 'PA');
INSERT INTO `r_transit` VALUES(3, 'Philadelphia, PA');
INSERT INTO `r_transit` VALUES(8, 'Pittsburgh, PA');
INSERT INTO `r_transit` VALUES(24, 'Portland, OR');
INSERT INTO `r_transit` VALUES(59, 'PR');
INSERT INTO `r_transit` VALUES(42, 'Pueblo, CO');
INSERT INTO `r_transit` VALUES(57, 'RI');
INSERT INTO `r_transit` VALUES(31, 'Salt Lake City, UT');
INSERT INTO `r_transit` VALUES(30, 'San Antonio, TX');
INSERT INTO `r_transit` VALUES(11, 'San Francisco, CA');
INSERT INTO `r_transit` VALUES(38, 'Savannah, GA');
INSERT INTO `r_transit` VALUES(67, 'SC');
INSERT INTO `r_transit` VALUES(78, 'SD');
INSERT INTO `r_transit` VALUES(19, 'Seattle, WA');
INSERT INTO `r_transit` VALUES(41, 'Sioux City, IA');
INSERT INTO `r_transit` VALUES(28, 'Spokane, WA');
INSERT INTO `r_transit` VALUES(36, 'St. Joseph, MO');
INSERT INTO `r_transit` VALUES(4, 'St. Louis, MO');
INSERT INTO `r_transit` VALUES(22, 'St. Paul, MN');
INSERT INTO `r_transit` VALUES(34, 'Tacoma, WA');
INSERT INTO `r_transit` VALUES(87, 'TN');
INSERT INTO `r_transit` VALUES(44, 'Topeka, KS');
INSERT INTO `r_transit` VALUES(88, 'TX');
INSERT INTO `r_transit` VALUES(97, 'UT');
INSERT INTO `r_transit` VALUES(68, 'VA');
INSERT INTO `r_transit` VALUES(59, 'VI');
INSERT INTO `r_transit` VALUES(58, 'VT');
INSERT INTO `r_transit` VALUES(98, 'WA');
INSERT INTO `r_transit` VALUES(48, 'Waco, TX');
INSERT INTO `r_transit` VALUES(15, 'Washington D.C.');
INSERT INTO `r_transit` VALUES(79, 'WI');
INSERT INTO `r_transit` VALUES(40, 'Wichita, KS');
INSERT INTO `r_transit` VALUES(69, 'WV');
INSERT INTO `r_transit` VALUES(99, 'WY');

-- --------------------------------------------------------

--
-- Table structure for table `r_usd2`
--

CREATE TABLE `r_usd2` (
  `id` bigint(20) NOT NULL COMMENT 'record ID',
  `type` char(1) DEFAULT NULL COMMENT 'transaction type (S=service charge, T=transfer between accounts)',
  `amount` decimal(11,2) DEFAULT NULL COMMENT 'amount of transfer',
  `completed` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime transaction was completed',
  `bankTxId` bigint(20) NOT NULL DEFAULT 0 COMMENT 'bank transaction ID',
  `memo` text DEFAULT NULL COMMENT 'transaction description (from bank)',
  `xid` bigint(20) NOT NULL DEFAULT 0 COMMENT 'id of related tx_hdrs record'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Record of transfers to or from a bank account' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `r_user_industries`
--

CREATE TABLE `r_user_industries` (
  `id` int(11) NOT NULL COMMENT 'user industry record id',
  `iid` int(11) NOT NULL DEFAULT 0 COMMENT 'industry id',
  `uid` bigint(20) NOT NULL DEFAULT 0 COMMENT 'industry id'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='industries for each company' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `r_votes`
--

CREATE TABLE `r_votes` (
  `id` bigint(20) NOT NULL COMMENT 'vote record id',
  `ballot` bigint(20) NOT NULL DEFAULT 0 COMMENT 'ballot on which this option is being graded',
  `option` bigint(20) NOT NULL DEFAULT 0 COMMENT 'option being graded',
  `grade` int(11) NOT NULL DEFAULT -1 COMMENT 'grade given by a particular voter for this option',
  `gradeMax` int(11) NOT NULL DEFAULT -1 COMMENT 'maximum grade given by a particular voter for this range-type option',
  `displayOrder` int(11) NOT NULL DEFAULT 0 COMMENT 'what order options were shown in to this voter',
  `text` longtext DEFAULT NULL COMMENT 'what was voter''s comment or moral objection to this option',
  `isVeto` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'this is a veto (not a canceled veto or mere comment)',
  `modified` int(11) NOT NULL DEFAULT 0 COMMENT 'date/time last modified'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='What grade a particular voter gave a particular options...' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `semaphore`
--

CREATE TABLE `semaphore` (
  `name` varchar(255) NOT NULL DEFAULT '' COMMENT 'Primary Key: Unique name.',
  `value` varchar(255) NOT NULL DEFAULT '' COMMENT 'A value for the semaphore.',
  `expire` double NOT NULL COMMENT 'A Unix timestamp with microseconds indicating when the semaphore should expire.'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Table for holding semaphores, locks, flags, etc. that...' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `sessions`
--

CREATE TABLE `sessions` (
  `uid` bigint(20) DEFAULT NULL COMMENT 'users record id',
  `acct` bigint(20) DEFAULT NULL COMMENT 'currently viewing/managing this account ID',
  `sid` varchar(128) NOT NULL COMMENT 'A session ID. The value is generated by Drupal’s session handlers.',
  `ssid` varchar(128) NOT NULL DEFAULT '' COMMENT 'Secure session ID. The value is generated by Drupal’s session handlers.',
  `hostname` varchar(128) NOT NULL DEFAULT '' COMMENT 'The IP address that last used this session ID (sid).',
  `timestamp` int(11) NOT NULL DEFAULT 0 COMMENT 'The Unix timestamp when this session last requested a page. Old records are purged by PHP automatically.',
  `cache` int(11) NOT NULL DEFAULT 0 COMMENT 'The time of this user’s last post. This is used when the site has specified a minimum_cache_lifetime. See cache_get().',
  `session` longblob DEFAULT NULL COMMENT 'The serialized contents of $_SESSION, an array of name/value pairs that persists across page requests by this session ID. Drupal loads $_SESSION from here at the start of each request and saves it at the end.'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Drupal’s session handlers read and write into the...' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `system`
--

CREATE TABLE `system` (
  `filename` varchar(255) NOT NULL DEFAULT '' COMMENT 'The path of the primary file for this item, relative to the Drupal root; e.g. modules/node/node.module.',
  `name` varchar(255) NOT NULL DEFAULT '' COMMENT 'The name of the item; e.g. node.',
  `type` varchar(12) NOT NULL DEFAULT '' COMMENT 'The type of the item, either module, theme, or theme_engine.',
  `owner` varchar(255) NOT NULL DEFAULT '' COMMENT 'A theme’s ’parent’ . Can be either a theme or an engine.',
  `status` int(11) NOT NULL DEFAULT 0 COMMENT 'Boolean indicating whether or not this item is enabled.',
  `bootstrap` int(11) NOT NULL DEFAULT 0 COMMENT 'Boolean indicating whether this module is loaded during Drupal’s early bootstrapping phase (e.g. even before the page cache is consulted).',
  `schema_version` smallint(6) NOT NULL DEFAULT -1 COMMENT 'The module’s database schema version number. -1 if the module is not installed (its tables do not exist); 0 or the largest N of the module’s hook_update_N() function that has either been run or existed when the module was first installed.',
  `weight` int(11) NOT NULL DEFAULT 0 COMMENT 'The order in which this module’s hooks should be invoked relative to other modules. Equal-weighted modules are ordered by name.',
  `info` blob DEFAULT NULL COMMENT 'A serialized array containing information from the module’s .info file; keys can include name, description, package, version, core, dependencies, and php.'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='A list of all modules, themes, and theme engines that are...' ROW_FORMAT=DYNAMIC;

--
-- Dumping data for table `system`
--

INSERT INTO `system` VALUES('modules/aggregator/aggregator.module', 'aggregator', 'module', '', 0, 0, -1, 0, 0x613a31343a7b733a343a226e616d65223b733a31303a2241676772656761746f72223b733a31313a226465736372697074696f6e223b733a35373a22416767726567617465732073796e6469636174656420636f6e74656e7420285253532c205244462c20616e642041746f6d206665656473292e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a353a2266696c6573223b613a313a7b693a303b733a31353a2261676772656761746f722e74657374223b7d733a393a22636f6e666967757265223b733a34313a2261646d696e2f636f6e6669672f73657276696365732f61676772656761746f722f73657474696e6773223b733a31313a227374796c65736865657473223b613a313a7b733a333a22616c6c223b613a313a7b733a31343a2261676772656761746f722e637373223b733a33333a226d6f64756c65732f61676772656761746f722f61676772656761746f722e637373223b7d7d733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a31323a22646570656e64656e63696573223b613a303a7b7d733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/block/block.module', 'block', 'module', '', 1, 0, 7009, -5, 0x613a31333a7b733a343a226e616d65223b733a353a22426c6f636b223b733a31313a226465736372697074696f6e223b733a3134303a22436f6e74726f6c73207468652076697375616c206275696c64696e6720626c6f636b732061207061676520697320636f6e737472756374656420776974682e20426c6f636b732061726520626f786573206f6620636f6e74656e742072656e646572656420696e746f20616e20617265612c206f7220726567696f6e2c206f6620612077656220706167652e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a353a2266696c6573223b613a313a7b693a303b733a31303a22626c6f636b2e74657374223b7d733a393a22636f6e666967757265223b733a32313a2261646d696e2f7374727563747572652f626c6f636b223b733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a31323a22646570656e64656e63696573223b613a303a7b7d733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/blog/blog.module', 'blog', 'module', '', 0, 0, -1, 0, 0x613a31323a7b733a343a226e616d65223b733a343a22426c6f67223b733a31313a226465736372697074696f6e223b733a32353a22456e61626c6573206d756c74692d7573657220626c6f67732e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a353a2266696c6573223b613a313a7b693a303b733a393a22626c6f672e74657374223b7d733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a31323a22646570656e64656e63696573223b613a303a7b7d733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/book/book.module', 'book', 'module', '', 0, 0, -1, 0, 0x613a31343a7b733a343a226e616d65223b733a343a22426f6f6b223b733a31313a226465736372697074696f6e223b733a36363a22416c6c6f777320757365727320746f2063726561746520616e64206f7267616e697a652072656c6174656420636f6e74656e7420696e20616e206f75746c696e652e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a353a2266696c6573223b613a313a7b693a303b733a393a22626f6f6b2e74657374223b7d733a393a22636f6e666967757265223b733a32373a2261646d696e2f636f6e74656e742f626f6f6b2f73657474696e6773223b733a31313a227374796c65736865657473223b613a313a7b733a333a22616c6c223b613a313a7b733a383a22626f6f6b2e637373223b733a32313a226d6f64756c65732f626f6f6b2f626f6f6b2e637373223b7d7d733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a31323a22646570656e64656e63696573223b613a303a7b7d733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/color/color.module', 'color', 'module', '', 0, 0, -1, 0, 0x613a31323a7b733a343a226e616d65223b733a353a22436f6c6f72223b733a31313a226465736372697074696f6e223b733a37303a22416c6c6f77732061646d696e6973747261746f727320746f206368616e67652074686520636f6c6f7220736368656d65206f6620636f6d70617469626c65207468656d65732e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a353a2266696c6573223b613a313a7b693a303b733a31303a22636f6c6f722e74657374223b7d733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a31323a22646570656e64656e63696573223b613a303a7b7d733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/comment/comment.module', 'comment', 'module', '', 0, 0, -1, 0, 0x613a31343a7b733a343a226e616d65223b733a373a22436f6d6d656e74223b733a31313a226465736372697074696f6e223b733a35373a22416c6c6f777320757365727320746f20636f6d6d656e74206f6e20616e642064697363757373207075626c697368656420636f6e74656e742e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a31323a22646570656e64656e63696573223b613a313a7b693a303b733a343a2274657874223b7d733a353a2266696c6573223b613a323a7b693a303b733a31343a22636f6d6d656e742e6d6f64756c65223b693a313b733a31323a22636f6d6d656e742e74657374223b7d733a393a22636f6e666967757265223b733a32313a2261646d696e2f636f6e74656e742f636f6d6d656e74223b733a31313a227374796c65736865657473223b613a313a7b733a333a22616c6c223b613a313a7b733a31313a22636f6d6d656e742e637373223b733a32373a226d6f64756c65732f636f6d6d656e742f636f6d6d656e742e637373223b7d7d733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/contact/contact.module', 'contact', 'module', '', 0, 0, -1, 0, 0x613a31333a7b733a343a226e616d65223b733a373a22436f6e74616374223b733a31313a226465736372697074696f6e223b733a36313a22456e61626c65732074686520757365206f6620626f746820706572736f6e616c20616e6420736974652d7769646520636f6e7461637420666f726d732e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a353a2266696c6573223b613a313a7b693a303b733a31323a22636f6e746163742e74657374223b7d733a393a22636f6e666967757265223b733a32333a2261646d696e2f7374727563747572652f636f6e74616374223b733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a31323a22646570656e64656e63696573223b613a303a7b7d733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/contextual/contextual.module', 'contextual', 'module', '', 0, 0, -1, 0, 0x613a31323a7b733a343a226e616d65223b733a31363a22436f6e7465787475616c206c696e6b73223b733a31313a226465736372697074696f6e223b733a37353a2250726f766964657320636f6e7465787475616c206c696e6b7320746f20706572666f726d20616374696f6e732072656c6174656420746f20656c656d656e7473206f6e206120706167652e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a353a2266696c6573223b613a313a7b693a303b733a31353a22636f6e7465787475616c2e74657374223b7d733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a31323a22646570656e64656e63696573223b613a303a7b7d733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/dashboard/dashboard.module', 'dashboard', 'module', '', 0, 0, -1, 0, 0x613a31333a7b733a343a226e616d65223b733a393a2244617368626f617264223b733a31313a226465736372697074696f6e223b733a3133363a2250726f766964657320612064617368626f617264207061676520696e207468652061646d696e69737472617469766520696e7465726661636520666f72206f7267616e697a696e672061646d696e697374726174697665207461736b7320616e6420747261636b696e6720696e666f726d6174696f6e2077697468696e20796f757220736974652e223b733a343a22636f7265223b733a333a22372e78223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a353a2266696c6573223b613a313a7b693a303b733a31343a2264617368626f6172642e74657374223b7d733a31323a22646570656e64656e63696573223b613a313a7b693a303b733a353a22626c6f636b223b7d733a393a22636f6e666967757265223b733a32353a2261646d696e2f64617368626f6172642f637573746f6d697a65223b733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/dblog/dblog.module', 'dblog', 'module', '', 0, 0, -1, 0, 0x613a31323a7b733a343a226e616d65223b733a31363a224461746162617365206c6f6767696e67223b733a31313a226465736372697074696f6e223b733a34373a224c6f677320616e64207265636f7264732073797374656d206576656e747320746f207468652064617461626173652e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a353a2266696c6573223b613a313a7b693a303b733a31303a2264626c6f672e74657374223b7d733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a31323a22646570656e64656e63696573223b613a303a7b7d733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/field_ui/field_ui.module', 'field_ui', 'module', '', 0, 0, -1, 0, 0x613a31323a7b733a343a226e616d65223b733a383a224669656c64205549223b733a31313a226465736372697074696f6e223b733a33333a225573657220696e7465726661636520666f7220746865204669656c64204150492e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a31323a22646570656e64656e63696573223b613a313a7b693a303b733a353a226669656c64223b7d733a353a2266696c6573223b613a313a7b693a303b733a31333a226669656c645f75692e74657374223b7d733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/field/field.module', 'field', 'module', '', 1, 0, 7003, 0, 0x613a31343a7b733a343a226e616d65223b733a353a224669656c64223b733a31313a226465736372697074696f6e223b733a35373a224669656c642041504920746f20616464206669656c647320746f20656e746974696573206c696b65206e6f64657320616e642075736572732e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a353a2266696c6573223b613a343a7b693a303b733a31323a226669656c642e6d6f64756c65223b693a313b733a31363a226669656c642e6174746163682e696e63223b693a323b733a32303a226669656c642e696e666f2e636c6173732e696e63223b693a333b733a31363a2274657374732f6669656c642e74657374223b7d733a31323a22646570656e64656e63696573223b613a313a7b693a303b733a31373a226669656c645f73716c5f73746f72616765223b7d733a383a227265717569726564223b623a313b733a31313a227374796c65736865657473223b613a313a7b733a333a22616c6c223b613a313a7b733a31353a227468656d652f6669656c642e637373223b733a32393a226d6f64756c65732f6669656c642f7468656d652f6669656c642e637373223b7d7d733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/field/modules/field_sql_storage/field_sql_storage.module', 'field_sql_storage', 'module', '', 1, 0, 7002, 0, 0x613a31333a7b733a343a226e616d65223b733a31373a224669656c642053514c2073746f72616765223b733a31313a226465736372697074696f6e223b733a33373a2253746f726573206669656c64206461746120696e20616e2053514c2064617461626173652e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a31323a22646570656e64656e63696573223b613a313a7b693a303b733a353a226669656c64223b7d733a353a2266696c6573223b613a313a7b693a303b733a32323a226669656c645f73716c5f73746f726167652e74657374223b7d733a383a227265717569726564223b623a313b733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/field/modules/list/list.module', 'list', 'module', '', 0, 0, -1, 0, 0x613a31323a7b733a343a226e616d65223b733a343a224c697374223b733a31313a226465736372697074696f6e223b733a36393a22446566696e6573206c697374206669656c642074797065732e205573652077697468204f7074696f6e7320746f206372656174652073656c656374696f6e206c697374732e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a31323a22646570656e64656e63696573223b613a323a7b693a303b733a353a226669656c64223b693a313b733a373a226f7074696f6e73223b7d733a353a2266696c6573223b613a313a7b693a303b733a31353a2274657374732f6c6973742e74657374223b7d733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/field/modules/number/number.module', 'number', 'module', '', 0, 0, -1, 0, 0x613a31323a7b733a343a226e616d65223b733a363a224e756d626572223b733a31313a226465736372697074696f6e223b733a32383a22446566696e6573206e756d65726963206669656c642074797065732e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a31323a22646570656e64656e63696573223b613a313a7b693a303b733a353a226669656c64223b7d733a353a2266696c6573223b613a313a7b693a303b733a31313a226e756d6265722e74657374223b7d733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/field/modules/options/options.module', 'options', 'module', '', 0, 0, -1, 0, 0x613a31323a7b733a343a226e616d65223b733a373a224f7074696f6e73223b733a31313a226465736372697074696f6e223b733a38323a22446566696e65732073656c656374696f6e2c20636865636b20626f7820616e6420726164696f20627574746f6e207769646765747320666f72207465787420616e64206e756d65726963206669656c64732e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a31323a22646570656e64656e63696573223b613a313a7b693a303b733a353a226669656c64223b7d733a353a2266696c6573223b613a313a7b693a303b733a31323a226f7074696f6e732e74657374223b7d733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/field/modules/text/text.module', 'text', 'module', '', 1, 0, 7000, 0, 0x613a31333a7b733a343a226e616d65223b733a343a2254657874223b733a31313a226465736372697074696f6e223b733a33323a22446566696e65732073696d706c652074657874206669656c642074797065732e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a31323a22646570656e64656e63696573223b613a313a7b693a303b733a353a226669656c64223b7d733a353a2266696c6573223b613a313a7b693a303b733a393a22746578742e74657374223b7d733a383a227265717569726564223b623a313b733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/file/file.module', 'file', 'module', '', 1, 0, 0, 0, 0x613a31323a7b733a343a226e616d65223b733a343a2246696c65223b733a31313a226465736372697074696f6e223b733a32363a22446566696e657320612066696c65206669656c6420747970652e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a31323a22646570656e64656e63696573223b613a313a7b693a303b733a353a226669656c64223b7d733a353a2266696c6573223b613a313a7b693a303b733a31353a2274657374732f66696c652e74657374223b7d733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/filter/filter.module', 'filter', 'module', '', 1, 0, 7010, 0, 0x613a31343a7b733a343a226e616d65223b733a363a2246696c746572223b733a31313a226465736372697074696f6e223b733a34333a2246696c7465727320636f6e74656e7420696e207072657061726174696f6e20666f7220646973706c61792e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a353a2266696c6573223b613a313a7b693a303b733a31313a2266696c7465722e74657374223b7d733a383a227265717569726564223b623a313b733a393a22636f6e666967757265223b733a32383a2261646d696e2f636f6e6669672f636f6e74656e742f666f726d617473223b733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a31323a22646570656e64656e63696573223b613a303a7b7d733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/forum/forum.module', 'forum', 'module', '', 0, 0, -1, 0, 0x613a31343a7b733a343a226e616d65223b733a353a22466f72756d223b733a31313a226465736372697074696f6e223b733a32373a2250726f76696465732064697363757373696f6e20666f72756d732e223b733a31323a22646570656e64656e63696573223b613a323a7b693a303b733a383a227461786f6e6f6d79223b693a313b733a373a22636f6d6d656e74223b7d733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a353a2266696c6573223b613a313a7b693a303b733a31303a22666f72756d2e74657374223b7d733a393a22636f6e666967757265223b733a32313a2261646d696e2f7374727563747572652f666f72756d223b733a31313a227374796c65736865657473223b613a313a7b733a333a22616c6c223b613a313a7b733a393a22666f72756d2e637373223b733a32333a226d6f64756c65732f666f72756d2f666f72756d2e637373223b7d7d733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/help/help.module', 'help', 'module', '', 0, 0, -1, 0, 0x613a31323a7b733a343a226e616d65223b733a343a2248656c70223b733a31313a226465736372697074696f6e223b733a33353a224d616e616765732074686520646973706c6179206f66206f6e6c696e652068656c702e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a353a2266696c6573223b613a313a7b693a303b733a393a2268656c702e74657374223b7d733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a31323a22646570656e64656e63696573223b613a303a7b7d733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/image/image.module', 'image', 'module', '', 0, 0, -1, 0, 0x613a31333a7b733a343a226e616d65223b733a353a22496d616765223b733a31313a226465736372697074696f6e223b733a33343a2250726f766964657320696d616765206d616e6970756c6174696f6e20746f6f6c732e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a31323a22646570656e64656e63696573223b613a313a7b693a303b733a343a2266696c65223b7d733a353a2266696c6573223b613a313a7b693a303b733a31303a22696d6167652e74657374223b7d733a393a22636f6e666967757265223b733a33313a2261646d696e2f636f6e6669672f6d656469612f696d6167652d7374796c6573223b733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/locale/locale.module', 'locale', 'module', '', 0, 0, -1, 0, 0x613a31333a7b733a343a226e616d65223b733a363a224c6f63616c65223b733a31313a226465736372697074696f6e223b733a3131393a2241646473206c616e67756167652068616e646c696e672066756e6374696f6e616c69747920616e6420656e61626c657320746865207472616e736c6174696f6e206f6620746865207573657220696e7465726661636520746f206c616e677561676573206f74686572207468616e20456e676c6973682e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a353a2266696c6573223b613a313a7b693a303b733a31313a226c6f63616c652e74657374223b7d733a393a22636f6e666967757265223b733a33303a2261646d696e2f636f6e6669672f726567696f6e616c2f6c616e6775616765223b733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a31323a22646570656e64656e63696573223b613a303a7b7d733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/menu/menu.module', 'menu', 'module', '', 0, 0, -1, 0, 0x613a31333a7b733a343a226e616d65223b733a343a224d656e75223b733a31313a226465736372697074696f6e223b733a36303a22416c6c6f77732061646d696e6973747261746f727320746f20637573746f6d697a65207468652073697465206e617669676174696f6e206d656e752e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a353a2266696c6573223b613a313a7b693a303b733a393a226d656e752e74657374223b7d733a393a22636f6e666967757265223b733a32303a2261646d696e2f7374727563747572652f6d656e75223b733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a31323a22646570656e64656e63696573223b613a303a7b7d733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/node/node.module', 'node', 'module', '', 1, 0, 7014, 0, 0x613a31353a7b733a343a226e616d65223b733a343a224e6f6465223b733a31313a226465736372697074696f6e223b733a36363a22416c6c6f777320636f6e74656e7420746f206265207375626d697474656420746f20746865207369746520616e6420646973706c61796564206f6e2070616765732e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a353a2266696c6573223b613a323a7b693a303b733a31313a226e6f64652e6d6f64756c65223b693a313b733a393a226e6f64652e74657374223b7d733a383a227265717569726564223b623a313b733a393a22636f6e666967757265223b733a32313a2261646d696e2f7374727563747572652f7479706573223b733a31313a227374796c65736865657473223b613a313a7b733a333a22616c6c223b613a313a7b733a383a226e6f64652e637373223b733a32313a226d6f64756c65732f6e6f64652f6e6f64652e637373223b7d7d733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a31323a22646570656e64656e63696573223b613a303a7b7d733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/openid/openid.module', 'openid', 'module', '', 0, 0, -1, 0, 0x613a31323a7b733a343a226e616d65223b733a363a224f70656e4944223b733a31313a226465736372697074696f6e223b733a34383a22416c6c6f777320757365727320746f206c6f6720696e746f20796f75722073697465207573696e67204f70656e49442e223b733a373a2276657273696f6e223b733a343a22372e3334223b733a373a227061636b616765223b733a343a22436f7265223b733a343a22636f7265223b733a333a22372e78223b733a353a2266696c6573223b613a313a7b693a303b733a31313a226f70656e69642e74657374223b7d733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a31323a22646570656e64656e63696573223b613a303a7b7d733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/overlay/overlay.module', 'overlay', 'module', '', 0, 0, -1, 0, 0x613a31323a7b733a343a226e616d65223b733a373a224f7665726c6179223b733a31313a226465736372697074696f6e223b733a35393a22446973706c617973207468652044727570616c2061646d696e697374726174696f6e20696e7465726661636520696e20616e206f7665726c61792e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a31323a22646570656e64656e63696573223b613a303a7b7d733a333a22706870223b733a353a22352e322e34223b733a353a2266696c6573223b613a303a7b7d733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/path/path.module', 'path', 'module', '', 0, 0, -1, 0, 0x613a31333a7b733a343a226e616d65223b733a343a2250617468223b733a31313a226465736372697074696f6e223b733a32383a22416c6c6f777320757365727320746f2072656e616d652055524c732e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a353a2266696c6573223b613a313a7b693a303b733a393a22706174682e74657374223b7d733a393a22636f6e666967757265223b733a32343a2261646d696e2f636f6e6669672f7365617263682f70617468223b733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a31323a22646570656e64656e63696573223b613a303a7b7d733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/php/php.module', 'php', 'module', '', 0, 0, -1, 0, 0x613a31323a7b733a343a226e616d65223b733a31303a225048502066696c746572223b733a31313a226465736372697074696f6e223b733a35303a22416c6c6f777320656d6265646465642050485020636f64652f736e69707065747320746f206265206576616c75617465642e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a353a2266696c6573223b613a313a7b693a303b733a383a227068702e74657374223b7d733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a31323a22646570656e64656e63696573223b613a303a7b7d733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/poll/poll.module', 'poll', 'module', '', 0, 0, -1, 0, 0x613a31333a7b733a343a226e616d65223b733a343a22506f6c6c223b733a31313a226465736372697074696f6e223b733a39353a22416c6c6f777320796f7572207369746520746f206361707475726520766f746573206f6e20646966666572656e7420746f7069637320696e2074686520666f726d206f66206d756c7469706c652063686f696365207175657374696f6e732e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a353a2266696c6573223b613a313a7b693a303b733a393a22706f6c6c2e74657374223b7d733a31313a227374796c65736865657473223b613a313a7b733a333a22616c6c223b613a313a7b733a383a22706f6c6c2e637373223b733a32313a226d6f64756c65732f706f6c6c2f706f6c6c2e637373223b7d7d733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a31323a22646570656e64656e63696573223b613a303a7b7d733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/profile/profile.module', 'profile', 'module', '', 0, 0, -1, 0, 0x613a31343a7b733a343a226e616d65223b733a373a2250726f66696c65223b733a31313a226465736372697074696f6e223b733a33363a22537570706f72747320636f6e666967757261626c6520757365722070726f66696c65732e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a353a2266696c6573223b613a313a7b693a303b733a31323a2270726f66696c652e74657374223b7d733a393a22636f6e666967757265223b733a32373a2261646d696e2f636f6e6669672f70656f706c652f70726f66696c65223b733a363a2268696464656e223b623a313b733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a31323a22646570656e64656e63696573223b613a303a7b7d733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/rdf/rdf.module', 'rdf', 'module', '', 0, 0, -1, 0, 0x613a31323a7b733a343a226e616d65223b733a333a22524446223b733a31313a226465736372697074696f6e223b733a3134383a22456e72696368657320796f757220636f6e74656e742077697468206d6574616461746120746f206c6574206f74686572206170706c69636174696f6e732028652e672e2073656172636820656e67696e65732c2061676772656761746f7273292062657474657220756e6465727374616e64206974732072656c6174696f6e736869707320616e6420617474726962757465732e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a353a2266696c6573223b613a313a7b693a303b733a383a227264662e74657374223b7d733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a31323a22646570656e64656e63696573223b613a303a7b7d733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/search/search.module', 'search', 'module', '', 0, 0, -1, 0, 0x613a31343a7b733a343a226e616d65223b733a363a22536561726368223b733a31313a226465736372697074696f6e223b733a33363a22456e61626c657320736974652d77696465206b6579776f726420736561726368696e672e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a353a2266696c6573223b613a323a7b693a303b733a31393a227365617263682e657874656e6465722e696e63223b693a313b733a31313a227365617263682e74657374223b7d733a393a22636f6e666967757265223b733a32383a2261646d696e2f636f6e6669672f7365617263682f73657474696e6773223b733a31313a227374796c65736865657473223b613a313a7b733a333a22616c6c223b613a313a7b733a31303a227365617263682e637373223b733a32353a226d6f64756c65732f7365617263682f7365617263682e637373223b7d7d733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a31323a22646570656e64656e63696573223b613a303a7b7d733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/shortcut/shortcut.module', 'shortcut', 'module', '', 0, 0, -1, 0, 0x613a31333a7b733a343a226e616d65223b733a383a2253686f7274637574223b733a31313a226465736372697074696f6e223b733a36303a22416c6c6f777320757365727320746f206d616e61676520637573746f6d697a61626c65206c69737473206f662073686f7274637574206c696e6b732e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a353a2266696c6573223b613a313a7b693a303b733a31333a2273686f72746375742e74657374223b7d733a393a22636f6e666967757265223b733a33363a2261646d696e2f636f6e6669672f757365722d696e746572666163652f73686f7274637574223b733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a31323a22646570656e64656e63696573223b613a303a7b7d733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/statistics/statistics.module', 'statistics', 'module', '', 0, 0, -1, 0, 0x613a31333a7b733a343a226e616d65223b733a31303a2253746174697374696373223b733a31313a226465736372697074696f6e223b733a33373a224c6f677320616363657373207374617469737469637320666f7220796f757220736974652e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a353a2266696c6573223b613a313a7b693a303b733a31353a22737461746973746963732e74657374223b7d733a393a22636f6e666967757265223b733a33303a2261646d696e2f636f6e6669672f73797374656d2f73746174697374696373223b733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a31323a22646570656e64656e63696573223b613a303a7b7d733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/syslog/syslog.module', 'syslog', 'module', '', 0, 0, -1, 0, 0x613a31333a7b733a343a226e616d65223b733a363a225379736c6f67223b733a31313a226465736372697074696f6e223b733a34313a224c6f677320616e64207265636f7264732073797374656d206576656e747320746f207379736c6f672e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a353a2266696c6573223b613a313a7b693a303b733a31313a227379736c6f672e74657374223b7d733a393a22636f6e666967757265223b733a33323a2261646d696e2f636f6e6669672f646576656c6f706d656e742f6c6f6767696e67223b733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a31323a22646570656e64656e63696573223b613a303a7b7d733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/system/system.module', 'system', 'module', '', 1, 0, 7079, 0, 0x613a31343a7b733a343a226e616d65223b733a363a2253797374656d223b733a31313a226465736372697074696f6e223b733a35343a2248616e646c65732067656e6572616c207369746520636f6e66696775726174696f6e20666f722061646d696e6973747261746f72732e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a353a2266696c6573223b613a363a7b693a303b733a31393a2273797374656d2e61726368697665722e696e63223b693a313b733a31353a2273797374656d2e6d61696c2e696e63223b693a323b733a31363a2273797374656d2e71756575652e696e63223b693a333b733a31343a2273797374656d2e7461722e696e63223b693a343b733a31383a2273797374656d2e757064617465722e696e63223b693a353b733a31313a2273797374656d2e74657374223b7d733a383a227265717569726564223b623a313b733a393a22636f6e666967757265223b733a31393a2261646d696e2f636f6e6669672f73797374656d223b733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a31323a22646570656e64656e63696573223b613a303a7b7d733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/taxonomy/taxonomy.module', 'taxonomy', 'module', '', 0, 0, -1, 0, 0x613a31333a7b733a343a226e616d65223b733a383a225461786f6e6f6d79223b733a31313a226465736372697074696f6e223b733a33383a22456e61626c6573207468652063617465676f72697a6174696f6e206f6620636f6e74656e742e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a31323a22646570656e64656e63696573223b613a313a7b693a303b733a373a226f7074696f6e73223b7d733a353a2266696c6573223b613a323a7b693a303b733a31353a227461786f6e6f6d792e6d6f64756c65223b693a313b733a31333a227461786f6e6f6d792e74657374223b7d733a393a22636f6e666967757265223b733a32343a2261646d696e2f7374727563747572652f7461786f6e6f6d79223b733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/toolbar/toolbar.module', 'toolbar', 'module', '', 0, 0, 0, 0, 0x613a31323a7b733a343a226e616d65223b733a373a22546f6f6c626172223b733a31313a226465736372697074696f6e223b733a39393a2250726f7669646573206120746f6f6c62617220746861742073686f77732074686520746f702d6c6576656c2061646d696e697374726174696f6e206d656e75206974656d7320616e64206c696e6b732066726f6d206f74686572206d6f64756c65732e223b733a343a22636f7265223b733a333a22372e78223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a31323a22646570656e64656e63696573223b613a303a7b7d733a333a22706870223b733a353a22352e322e34223b733a353a2266696c6573223b613a303a7b7d733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/tracker/tracker.module', 'tracker', 'module', '', 0, 0, -1, 0, 0x613a31323a7b733a343a226e616d65223b733a373a22547261636b6572223b733a31313a226465736372697074696f6e223b733a34353a22456e61626c657320747261636b696e67206f6620726563656e7420636f6e74656e7420666f722075736572732e223b733a31323a22646570656e64656e63696573223b613a313a7b693a303b733a373a22636f6d6d656e74223b7d733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a353a2266696c6573223b613a313a7b693a303b733a31323a22747261636b65722e74657374223b7d733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/translation/translation.module', 'translation', 'module', '', 0, 0, -1, 0, 0x613a31323a7b733a343a226e616d65223b733a31393a22436f6e74656e74207472616e736c6174696f6e223b733a31313a226465736372697074696f6e223b733a35373a22416c6c6f777320636f6e74656e7420746f206265207472616e736c6174656420696e746f20646966666572656e74206c616e6775616765732e223b733a31323a22646570656e64656e63696573223b613a313a7b693a303b733a363a226c6f63616c65223b7d733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a353a2266696c6573223b613a313a7b693a303b733a31363a227472616e736c6174696f6e2e74657374223b7d733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/trigger/trigger.module', 'trigger', 'module', '', 0, 0, -1, 0, 0x613a31333a7b733a343a226e616d65223b733a373a2254726967676572223b733a31313a226465736372697074696f6e223b733a39303a22456e61626c657320616374696f6e7320746f206265206669726564206f6e206365727461696e2073797374656d206576656e74732c2073756368206173207768656e206e657720636f6e74656e7420697320637265617465642e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a353a2266696c6573223b613a313a7b693a303b733a31323a22747269676765722e74657374223b7d733a393a22636f6e666967757265223b733a32333a2261646d696e2f7374727563747572652f74726967676572223b733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a31323a22646570656e64656e63696573223b613a303a7b7d733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/update/update.module', 'update', 'module', '', 1, 0, 7001, 0, 0x613a31333a7b733a343a226e616d65223b733a31343a22557064617465206d616e61676572223b733a31313a226465736372697074696f6e223b733a3130343a22436865636b7320666f7220617661696c61626c6520757064617465732c20616e642063616e207365637572656c7920696e7374616c6c206f7220757064617465206d6f64756c657320616e64207468656d65732076696120612077656220696e746572666163652e223b733a373a2276657273696f6e223b733a343a22372e3334223b733a373a227061636b616765223b733a343a22436f7265223b733a343a22636f7265223b733a333a22372e78223b733a353a2266696c6573223b613a313a7b693a303b733a31313a227570646174652e74657374223b7d733a393a22636f6e666967757265223b733a33303a2261646d696e2f7265706f7274732f757064617465732f73657474696e6773223b733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a31323a22646570656e64656e63696573223b613a303a7b7d733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('modules/user/user.module', 'user', 'module', '', 0, 0, 7018, 0, 0x613a31353a7b733a343a226e616d65223b733a343a2255736572223b733a31313a226465736372697074696f6e223b733a34373a224d616e6167657320746865207573657220726567697374726174696f6e20616e64206c6f67696e2073797374656d2e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a353a2266696c6573223b613a323a7b693a303b733a31313a22757365722e6d6f64756c65223b693a313b733a393a22757365722e74657374223b7d733a383a227265717569726564223b623a313b733a393a22636f6e666967757265223b733a31393a2261646d696e2f636f6e6669672f70656f706c65223b733a31313a227374796c65736865657473223b613a313a7b733a333a22616c6c223b613a313a7b733a383a22757365722e637373223b733a32313a226d6f64756c65732f757365722f757365722e637373223b7d7d733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a31323a22646570656e64656e63696573223b613a303a7b7d733a333a22706870223b733a353a22352e322e34223b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('profiles/standard/standard.profile', 'standard', 'module', '', 1, 0, 0, 1000, 0x613a31353a7b733a343a226e616d65223b733a383a225374616e64617264223b733a31313a226465736372697074696f6e223b733a35313a22496e7374616c6c207769746820636f6d6d6f6e6c792075736564206665617475726573207072652d636f6e666967757265642e223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a31323a22646570656e64656e63696573223b613a32313a7b693a303b733a353a22626c6f636b223b693a313b733a353a22636f6c6f72223b693a323b733a373a22636f6d6d656e74223b693a333b733a31303a22636f6e7465787475616c223b693a343b733a393a2264617368626f617264223b693a353b733a343a2268656c70223b693a363b733a353a22696d616765223b693a373b733a343a226c697374223b693a383b733a343a226d656e75223b693a393b733a363a226e756d626572223b693a31303b733a373a226f7074696f6e73223b693a31313b733a343a2270617468223b693a31323b733a383a227461786f6e6f6d79223b693a31333b733a353a2264626c6f67223b693a31343b733a363a22736561726368223b693a31353b733a383a2273686f7274637574223b693a31363b733a373a22746f6f6c626172223b693a31373b733a373a226f7665726c6179223b693a31383b733a383a226669656c645f7569223b693a31393b733a343a2266696c65223b693a32303b733a333a22726466223b7d733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a353a226d74696d65223b693a313431363432393438383b733a373a227061636b616765223b733a353a224f74686572223b733a333a22706870223b733a353a22352e322e34223b733a353a2266696c6573223b613a303a7b7d733a393a22626f6f747374726170223b693a303b733a363a2268696464656e223b623a313b733a383a227265717569726564223b623a313b733a31373a22646973747269627574696f6e5f6e616d65223b733a363a2244727570616c223b7d);
INSERT INTO `system` VALUES('rcredits/rcredits.module', 'rcredits', 'module', '', 1, 1, 0, 0, 0x613a31303a7b733a343a226e616d65223b733a383a227243726564697473223b733a31313a226465736372697074696f6e223b733a36353a22436f7265207472616e73616374696f6e2070726f63657373696e6720666f7220746865207243726564697473206d757475616c206372656469742073797374656d223b733a373a227061636b616765223b733a383a227243726564697473223b733a343a22636f7265223b733a333a22372e78223b733a333a22706870223b733a333a22352e33223b733a353a2266696c6573223b613a393a7b693a303b733a31353a2272637265646974732e6d6f64756c65223b693a313b733a31363a2272637265646974732e696e7374616c6c223b693a323b733a32313a2272637265646974732d73657474696e67732e696e63223b693a333b733a393a227263726f6e2e696e63223b693a343b733a31373a2272637265646974732d7574696c2e696e63223b693a353b733a31323a2272637265646974732e696e63223b693a363b733a32303a2272637265646974732d6261636b656e642e696e63223b693a373b733a31353a2272637265646974732d64622e696e63223b693a383b733a31353a2261646d696e2f61646d696e2e696e63223b7d733a353a226d74696d65223b693a313431383538363032343b733a31323a22646570656e64656e63696573223b613a303a7b7d733a373a2276657273696f6e223b4e3b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('rcredits/rsmart/rsmart.module', 'rsmart', 'module', '', 1, 0, 0, 0, 0x613a31303a7b733a343a226e616d65223b733a33313a22724372656469747320536d6172742044657669636520496e74657266616365223b733a31313a226465736372697074696f6e223b733a34363a22536d6172742064657669636520496e7465726661636520746f207468652072437265646974732073797374656d2e223b733a373a227061636b616765223b733a383a227243726564697473223b733a343a22636f7265223b733a333a22372e78223b733a333a22706870223b733a333a22352e33223b733a31323a22646570656e64656e63696573223b613a313a7b693a303b733a383a227263726564697473223b7d733a353a2266696c6573223b613a32393a7b693a303b733a31333a2272736d6172742e6d6f64756c65223b693a313b733a31303a2272736d6172742e696e63223b693a323b733a32313a2274657374732f4964656e7469667951522e74657374223b693a333b733a31383a2274657374732f537461727475702e74657374223b693a343b733a31393a2274657374732f5472616e736163742e74657374223b693a353b733a33333a2274657374732f5472616e736163744d656d626572546f4d656d6265722e74657374223b693a363b733a31353a2274657374732f556e646f2e74657374223b693a373b733a32343a2274657374732f556e646f436f6d706c657465642e74657374223b693a383b733a32323a2274657374732f556e646f50656e64696e672e74657374223b693a393b733a32313a2274657374732f556e646f41747461636b2e74657374223b693a31303b733a32333a2274657374732f496e73756666696369656e742e74657374223b693a31313b733a31373a2274657374732f4368616e67652e74657374223b693a31323b733a31363a2274657374732f6164686f632e74657374223b693a31333b733a32313a2274657374732f556e696c61746572616c2e74657374223b693a31343b733a32373a2274657374732f4964656e74696679437573746f6d65722e74657374223b693a31353b733a31393a2274657374732f4964656e746966792e74657374223b693a31363b733a31393a2274657374732f45786368616e67652e74657374223b693a31373b733a31353a2274657374732f54696d652e74657374223b693a31383b733a31383a2274657374732f4f66666c696e652e74657374223b693a31393b733a31383a2274657374732f5370656369616c2e74657374223b693a32303b733a31363a2274657374732f4a6f696e742e74657374223b693a32313b733a31383a22746573742f45786368616e67652e74657374223b693a32323b733a31383a22746573742f4964656e746966792e74657374223b693a32333b733a31353a22746573742f4a6f696e742e74657374223b693a32343b733a31373a22746573742f4f66666c696e652e74657374223b693a32353b733a31373a22746573742f537461727475702e74657374223b693a32363b733a31343a22746573742f54696d652e74657374223b693a32373b733a31383a22746573742f5472616e736163742e74657374223b693a32383b733a31343a22746573742f556e646f2e74657374223b7d733a353a226d74696d65223b693a313436363731373232323b733a373a2276657273696f6e223b4e3b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('rcredits/rsms/rsms.module', 'rsms', 'module', '', 1, 0, 0, 0, 0x613a31303a7b733a343a226e616d65223b733a32323a22724372656469747320534d5320696e74657266616365223b733a31313a226465736372697074696f6e223b733a38343a22526571756573742065786368616e6765732c20696e666f726d6174696f6e2c20616e64206f74686572206f7065726174696f6e732066726f6d20796f75722063656c6c2070686f6e652c207573696e6720534d53223b733a373a227061636b616765223b733a383a227243726564697473223b733a343a22636f7265223b733a333a22372e78223b733a333a22706870223b733a333a22352e33223b733a31323a22646570656e64656e63696573223b613a313a7b693a303b733a383a227263726564697473223b7d733a353a2266696c6573223b613a32313a7b693a303b733a31323a2272736d732e696e7374616c6c223b693a313b733a31313a2272736d732e6d6f64756c65223b693a323b733a383a2272736d732e696e63223b693a333b733a31333a2272736d732d63616c6c2e696e63223b693a343b733a32303a222e2e2f72637265646974732d7574696c2e696e63223b693a353b733a31393a222e2e2f72637265646974732d6170692e696e63223b693a363b733a31303a2272736d732e7374657073223b693a373b733a32313a22676865726b696e2f746573745f646566732e706870223b693a383b733a32383a2274657374732f416262726576696174696f6e73576f726b2e74657374223b693a393b733a32363a2274657374732f45786368616e6765466f72436173682e74657374223b693a31303b733a31383a2274657374732f47657448656c702e74657374223b693a31313b733a32353a2274657374732f476574496e666f726d6174696f6e2e74657374223b693a31323b733a34363a2274657374732f4f66666572546f45786368616e67655553446f6c6c617273466f7252437265646974732e74657374223b693a31333b733a33363a2274657374732f4f70656e416e4163636f756e74466f7254686543616c6c65722e74657374223b693a31343b733a31393a2274657374732f5472616e736163742e74657374223b693a31353b733a31353a2274657374732f556e646f2e74657374223b693a31363b733a32343a2274657374732f416262726576696174696f6e732e74657374223b693a31373b733a31353a2274657374732f42616e6b2e74657374223b693a31383b733a31393a2274657374732f45786368616e67652e74657374223b693a31393b733a31353a2274657374732f48656c702e74657374223b693a32303b733a32323a2274657374732f496e666f726d6174696f6e2e74657374223b7d733a353a226d74696d65223b693a313430373037313939363b733a373a2276657273696f6e223b4e3b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('rcredits/rweb/rweb.module', 'rweb', 'module', '', 1, 0, 0, 0, 0x613a31303a7b733a343a226e616d65223b733a32323a2272437265646974732057656220496e74657266616365223b733a31313a226465736372697074696f6e223b733a34353a225765622042726f7773657220496e7465726661636520746f207468652072437265646974732073797374656d2e223b733a373a227061636b616765223b733a383a227243726564697473223b733a343a22636f7265223b733a333a22372e78223b733a333a22706870223b733a333a22352e33223b733a31323a22646570656e64656e63696573223b613a313a7b693a303b733a383a227263726564697473223b7d733a353a2266696c6573223b613a36343a7b693a303b733a31313a22727765622e6d6f64756c65223b693a313b733a383a22727765622e696e63223b693a323b733a31323a22727765622d7478732e696e63223b693a333b733a32323a222e2e2f61646d696e2f61646d696e2d7765622e696e63223b693a343b733a32343a222e2e2f61646d696e2f61646d696e2d666f726d732e696e63223b693a353b733a32383a2274657374732f414d656d6265724861734f7074696f6e732e74657374223b693a363b733a32363a2274657374732f45786368616e6765466f72436173682e74657374223b693a373b733a31383a2274657374732f47657448656c702e74657374223b693a383b733a32353a2274657374732f476574496e666f726d6174696f6e2e74657374223b693a393b733a34363a2274657374732f4f66666572546f45786368616e67655553446f6c6c617273466f7252437265646974732e74657374223b693a31303b733a33363a2274657374732f4f70656e416e4163636f756e74466f7254686543616c6c65722e74657374223b693a31313b733a31373a2274657374732f5369676e75702e74657374223b693a31323b733a32323a2274657374732f5472616e73616374696f6e2e74657374223b693a31333b733a31353a2274657374732f556e646f2e74657374223b693a31343b733a31393a2274657374732f5472616e736163742e74657374223b693a31353b733a31383a2274657374732f53756d6d6172792e74657374223b693a31363b733a32333a2274657374732f5472616e73616374696f6e732e74657374223b693a31373b733a32303a2274657374732f52656c6174696f6e732e74657374223b693a31383b733a31363a2274657374732f6d756c74692e74657374223b693a31393b733a32313a2274657374732f4d656d626572736869702e74657374223b693a32303b733a31383a2274657374732f5265616c5573642e74657374223b693a32313b733a32303a2274657374732f5472616e73616374522e74657374223b693a32323b733a32323a2274657374732f5472616e736163745573642e74657374223b693a32333b733a31373a2274657374732f5369676e696e2e74657374223b693a32343b733a31353a2274657374732f476966742e74657374223b693a32353b733a31353a2274657374732f466c6f772e74657374223b693a32363b733a31393a2274657374732f446f776e6c6f61642e74657374223b693a32373b733a31343a2274657374732f4765742e74657374223b693a32383b733a32373a2274657374732f496e636f6d706c6574655573645478732e74657374223b693a32393b733a31393a2274657374732f5363616e436172642e74657374223b693a33303b733a31383a2274657374732f436f6e746163742e74657374223b693a33313b733a32313a2274657374732f537461746973746963732e74657374223b693a33323b733a32303a2274657374732f436f6d6d756e6974792e74657374223b693a33333b733a31363a2274657374732f426f7865732e74657374223b693a33343b733a31353a2274657374732f42616e6b2e74657374223b693a33353b733a31373a2274657374732f4564697454782e74657374223b693a33363b733a31383a2274657374732f436f6d70616e792e74657374223b693a33373b733a32323a2274657374732f4d656d62657273686970322e74657374223b693a33383b733a31363a2274657374732f4a6f696e742e74657374223b693a33393b733a31393a2274657374732f45786368616e67652e74657374223b693a34303b733a32303a2274657374732f5265696d62757273652e74657374223b693a34313b733a31393a2274657374732f5369676e7570436f2e74657374223b693a34323b733a32323a2274657374732f507265666572656e6365732e74657374223b693a34333b733a31343a22746573742f42616e6b2e74657374223b693a34343b733a31393a22746573742f436f6d6d756e6974792e74657374223b693a34353b733a31373a22746573742f436f6d70616e792e74657374223b693a34363b733a31373a22746573742f436f6e746163742e74657374223b693a34373b733a31383a22746573742f446f776e6c6f61642e74657374223b693a34383b733a31363a22746573742f4564697454782e74657374223b693a34393b733a31383a22746573742f45786368616e67652e74657374223b693a35303b733a31343a22746573742f466c6f772e74657374223b693a35313b733a31343a22746573742f476966742e74657374223b693a35323b733a31353a22746573742f4a6f696e742e74657374223b693a35333b733a32303a22746573742f4d656d626572736869702e74657374223b693a35343b733a32313a22746573742f507265666572656e6365732e74657374223b693a35353b733a31393a22746573742f5265696d62757273652e74657374223b693a35363b733a31393a22746573742f52656c6174696f6e732e74657374223b693a35373b733a31383a22746573742f5363616e436172642e74657374223b693a35383b733a31363a22746573742f5369676e696e2e74657374223b693a35393b733a31363a22746573742f5369676e75702e74657374223b693a36303b733a31383a22746573742f5369676e7570436f2e74657374223b693a36313b733a31373a22746573742f53756d6d6172792e74657374223b693a36323b733a31383a22746573742f5472616e736163742e74657374223b693a36333b733a32323a22746573742f5472616e73616374696f6e732e74657374223b7d733a353a226d74696d65223b693a313436363731373232383b733a373a2276657273696f6e223b4e3b733a393a22626f6f747374726170223b693a303b7d);
INSERT INTO `system` VALUES('rcredits/theme/theme.info', 'rcredits', 'theme', 'themes/engines/phptemplate/phptemplate.engine', 1, 1, 0, 0, 0x613a31343a7b733a343a226e616d65223b733a383a227243726564697473223b733a31313a226465736372697074696f6e223b733a38303a224120766172696174696f6e206f6e20526573706f6e736976652042617274696b20287072652d72656c6561736520323031322d3038292c20666f72207468652072437265646974732053797374656d2e223b733a373a2276657273696f6e223b733a333a22312e30223b733a343a22636f7265223b733a333a22372e78223b733a31313a227374796c65736865657473223b613a323a7b733a333a22616c6c223b613a333a7b733a31343a226373732f6c61796f75742e637373223b733a32393a2272637265646974732f7468656d652f6373732f6c61796f75742e637373223b733a31333a226373732f7374796c652e637373223b733a32383a2272637265646974732f7468656d652f6373732f7374796c652e637373223b733a31343a226373732f636f6c6f72732e637373223b733a32393a2272637265646974732f7468656d652f6373732f636f6c6f72732e637373223b7d733a353a227072696e74223b613a313a7b733a31333a226373732f7072696e742e637373223b733a32383a2272637265646974732f7468656d652f6373732f7072696e742e637373223b7d7d733a373a22726567696f6e73223b613a31383a7b733a363a22686561646572223b733a363a22486561646572223b733a343a2268656c70223b733a343a2248656c70223b733a383a22706167655f746f70223b733a383a225061676520746f70223b733a31313a22706167655f626f74746f6d223b733a31313a225061676520626f74746f6d223b733a31313a22686967686c696768746564223b733a31313a22486967686c696768746564223b733a383a226163636f756e7473223b733a31363a224163636f756e742073656c6563746f72223b733a383a226665617475726564223b733a383a224665617475726564223b733a373a22636f6e74656e74223b733a373a22436f6e74656e74223b733a31333a22736964656261725f6669727374223b733a31333a2253696465626172206669727374223b733a31343a22736964656261725f7365636f6e64223b733a31343a2253696465626172207365636f6e64223b733a31343a2274726970747963685f6669727374223b733a31343a225472697074796368206669727374223b733a31353a2274726970747963685f6d6964646c65223b733a31353a225472697074796368206d6964646c65223b733a31333a2274726970747963685f6c617374223b733a31333a225472697074796368206c617374223b733a31383a22666f6f7465725f6669727374636f6c756d6e223b733a31393a22466f6f74657220666972737420636f6c756d6e223b733a31393a22666f6f7465725f7365636f6e64636f6c756d6e223b733a32303a22466f6f746572207365636f6e6420636f6c756d6e223b733a31383a22666f6f7465725f7468697264636f6c756d6e223b733a31393a22466f6f74657220746869726420636f6c756d6e223b733a31393a22666f6f7465725f666f75727468636f6c756d6e223b733a32303a22466f6f74657220666f7572746820636f6c756d6e223b733a363a22666f6f746572223b733a363a22466f6f746572223b7d733a383a2273657474696e6773223b613a313a7b733a32303a2273686f72746375745f6d6f64756c655f6c696e6b223b733a313a2230223b7d733a363a22656e67696e65223b733a31313a2270687074656d706c617465223b733a383a226665617475726573223b613a393a7b693a303b733a343a226c6f676f223b693a313b733a373a2266617669636f6e223b693a323b733a343a226e616d65223b693a333b733a363a22736c6f67616e223b693a343b733a31373a226e6f64655f757365725f70696374757265223b693a353b733a32303a22636f6d6d656e745f757365725f70696374757265223b693a363b733a32353a22636f6d6d656e745f757365725f766572696669636174696f6e223b693a373b733a393a226d61696e5f6d656e75223b693a383b733a31343a227365636f6e646172795f6d656e75223b7d733a31303a2273637265656e73686f74223b733a34303a2273697465732f616c6c2f7468656d65732f72637265646974732f73637265656e73686f742e706e67223b733a333a22706870223b733a353a22352e322e34223b733a373a2273637269707473223b613a303a7b7d733a353a226d74696d65223b693a313335343831383432363b733a31343a22726567696f6e735f68696464656e223b613a323a7b693a303b733a383a22706167655f746f70223b693a313b733a31313a22706167655f626f74746f6d223b7d7d);
INSERT INTO `system` VALUES('themes/bartik/bartik.info', 'bartik', 'theme', 'themes/engines/phptemplate/phptemplate.engine', 0, 0, -1, 0, 0x613a31373a7b733a343a226e616d65223b733a363a2242617274696b223b733a31313a226465736372697074696f6e223b733a34383a224120666c657869626c652c207265636f6c6f7261626c65207468656d652077697468206d616e7920726567696f6e732e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a31313a227374796c65736865657473223b613a323a7b733a333a22616c6c223b613a333a7b733a31343a226373732f6c61796f75742e637373223b733a32383a227468656d65732f62617274696b2f6373732f6c61796f75742e637373223b733a31333a226373732f7374796c652e637373223b733a32373a227468656d65732f62617274696b2f6373732f7374796c652e637373223b733a31343a226373732f636f6c6f72732e637373223b733a32383a227468656d65732f62617274696b2f6373732f636f6c6f72732e637373223b7d733a353a227072696e74223b613a313a7b733a31333a226373732f7072696e742e637373223b733a32373a227468656d65732f62617274696b2f6373732f7072696e742e637373223b7d7d733a373a22726567696f6e73223b613a31373a7b733a363a22686561646572223b733a363a22486561646572223b733a343a2268656c70223b733a343a2248656c70223b733a383a22706167655f746f70223b733a383a225061676520746f70223b733a31313a22706167655f626f74746f6d223b733a31313a225061676520626f74746f6d223b733a31313a22686967686c696768746564223b733a31313a22486967686c696768746564223b733a383a226665617475726564223b733a383a224665617475726564223b733a373a22636f6e74656e74223b733a373a22436f6e74656e74223b733a31333a22736964656261725f6669727374223b733a31333a2253696465626172206669727374223b733a31343a22736964656261725f7365636f6e64223b733a31343a2253696465626172207365636f6e64223b733a31343a2274726970747963685f6669727374223b733a31343a225472697074796368206669727374223b733a31353a2274726970747963685f6d6964646c65223b733a31353a225472697074796368206d6964646c65223b733a31333a2274726970747963685f6c617374223b733a31333a225472697074796368206c617374223b733a31383a22666f6f7465725f6669727374636f6c756d6e223b733a31393a22466f6f74657220666972737420636f6c756d6e223b733a31393a22666f6f7465725f7365636f6e64636f6c756d6e223b733a32303a22466f6f746572207365636f6e6420636f6c756d6e223b733a31383a22666f6f7465725f7468697264636f6c756d6e223b733a31393a22466f6f74657220746869726420636f6c756d6e223b733a31393a22666f6f7465725f666f75727468636f6c756d6e223b733a32303a22466f6f74657220666f7572746820636f6c756d6e223b733a363a22666f6f746572223b733a363a22466f6f746572223b7d733a383a2273657474696e6773223b613a313a7b733a32303a2273686f72746375745f6d6f64756c655f6c696e6b223b733a313a2230223b7d733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a363a22656e67696e65223b733a31313a2270687074656d706c617465223b733a383a226665617475726573223b613a393a7b693a303b733a343a226c6f676f223b693a313b733a373a2266617669636f6e223b693a323b733a343a226e616d65223b693a333b733a363a22736c6f67616e223b693a343b733a31373a226e6f64655f757365725f70696374757265223b693a353b733a32303a22636f6d6d656e745f757365725f70696374757265223b693a363b733a32353a22636f6d6d656e745f757365725f766572696669636174696f6e223b693a373b733a393a226d61696e5f6d656e75223b693a383b733a31343a227365636f6e646172795f6d656e75223b7d733a31303a2273637265656e73686f74223b733a32383a227468656d65732f62617274696b2f73637265656e73686f742e706e67223b733a333a22706870223b733a353a22352e322e34223b733a373a2273637269707473223b613a303a7b7d733a353a226d74696d65223b693a313431363432393438383b733a31343a22726567696f6e735f68696464656e223b613a323a7b693a303b733a383a22706167655f746f70223b693a313b733a31313a22706167655f626f74746f6d223b7d7d);
INSERT INTO `system` VALUES('themes/garland/garland.info', 'garland', 'theme', 'themes/engines/phptemplate/phptemplate.engine', 0, 0, -1, 0, 0x613a31373a7b733a343a226e616d65223b733a373a224761726c616e64223b733a31313a226465736372697074696f6e223b733a3131313a2241206d756c74692d636f6c756d6e207468656d652077686963682063616e20626520636f6e6669677572656420746f206d6f6469667920636f6c6f727320616e6420737769746368206265747765656e20666978656420616e6420666c756964207769647468206c61796f7574732e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a31313a227374796c65736865657473223b613a323a7b733a333a22616c6c223b613a313a7b733a393a227374796c652e637373223b733a32343a227468656d65732f6761726c616e642f7374796c652e637373223b7d733a353a227072696e74223b613a313a7b733a393a227072696e742e637373223b733a32343a227468656d65732f6761726c616e642f7072696e742e637373223b7d7d733a383a2273657474696e6773223b613a313a7b733a31333a226761726c616e645f7769647468223b733a353a22666c756964223b7d733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a363a22656e67696e65223b733a31313a2270687074656d706c617465223b733a373a22726567696f6e73223b613a393a7b733a31333a22736964656261725f6669727374223b733a31323a224c6566742073696465626172223b733a31343a22736964656261725f7365636f6e64223b733a31333a2252696768742073696465626172223b733a373a22636f6e74656e74223b733a373a22436f6e74656e74223b733a363a22686561646572223b733a363a22486561646572223b733a363a22666f6f746572223b733a363a22466f6f746572223b733a31313a22686967686c696768746564223b733a31313a22486967686c696768746564223b733a343a2268656c70223b733a343a2248656c70223b733a383a22706167655f746f70223b733a383a225061676520746f70223b733a31313a22706167655f626f74746f6d223b733a31313a225061676520626f74746f6d223b7d733a383a226665617475726573223b613a393a7b693a303b733a343a226c6f676f223b693a313b733a373a2266617669636f6e223b693a323b733a343a226e616d65223b693a333b733a363a22736c6f67616e223b693a343b733a31373a226e6f64655f757365725f70696374757265223b693a353b733a32303a22636f6d6d656e745f757365725f70696374757265223b693a363b733a32353a22636f6d6d656e745f757365725f766572696669636174696f6e223b693a373b733a393a226d61696e5f6d656e75223b693a383b733a31343a227365636f6e646172795f6d656e75223b7d733a31303a2273637265656e73686f74223b733a32393a227468656d65732f6761726c616e642f73637265656e73686f742e706e67223b733a333a22706870223b733a353a22352e322e34223b733a373a2273637269707473223b613a303a7b7d733a353a226d74696d65223b693a313431363432393438383b733a31343a22726567696f6e735f68696464656e223b613a323a7b693a303b733a383a22706167655f746f70223b693a313b733a31313a22706167655f626f74746f6d223b7d7d);
INSERT INTO `system` VALUES('themes/seven/seven.info', 'seven', 'theme', 'themes/engines/phptemplate/phptemplate.engine', 1, 0, -1, 0, 0x613a31373a7b733a343a226e616d65223b733a353a22536576656e223b733a31313a226465736372697074696f6e223b733a36353a22412073696d706c65206f6e652d636f6c756d6e2c207461626c656c6573732c20666c7569642077696474682061646d696e697374726174696f6e207468656d652e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a31313a227374796c65736865657473223b613a313a7b733a363a2273637265656e223b613a323a7b733a393a2272657365742e637373223b733a32323a227468656d65732f736576656e2f72657365742e637373223b733a393a227374796c652e637373223b733a32323a227468656d65732f736576656e2f7374796c652e637373223b7d7d733a383a2273657474696e6773223b613a313a7b733a32303a2273686f72746375745f6d6f64756c655f6c696e6b223b733a313a2231223b7d733a373a22726567696f6e73223b613a353a7b733a373a22636f6e74656e74223b733a373a22436f6e74656e74223b733a343a2268656c70223b733a343a2248656c70223b733a383a22706167655f746f70223b733a383a225061676520746f70223b733a31313a22706167655f626f74746f6d223b733a31313a225061676520626f74746f6d223b733a31333a22736964656261725f6669727374223b733a31333a2246697273742073696465626172223b7d733a31343a22726567696f6e735f68696464656e223b613a333a7b693a303b733a31333a22736964656261725f6669727374223b693a313b733a383a22706167655f746f70223b693a323b733a31313a22706167655f626f74746f6d223b7d733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a363a22656e67696e65223b733a31313a2270687074656d706c617465223b733a383a226665617475726573223b613a393a7b693a303b733a343a226c6f676f223b693a313b733a373a2266617669636f6e223b693a323b733a343a226e616d65223b693a333b733a363a22736c6f67616e223b693a343b733a31373a226e6f64655f757365725f70696374757265223b693a353b733a32303a22636f6d6d656e745f757365725f70696374757265223b693a363b733a32353a22636f6d6d656e745f757365725f766572696669636174696f6e223b693a373b733a393a226d61696e5f6d656e75223b693a383b733a31343a227365636f6e646172795f6d656e75223b7d733a31303a2273637265656e73686f74223b733a32373a227468656d65732f736576656e2f73637265656e73686f742e706e67223b733a333a22706870223b733a353a22352e322e34223b733a373a2273637269707473223b613a303a7b7d733a353a226d74696d65223b693a313431363432393438383b7d);
INSERT INTO `system` VALUES('themes/stark/stark.info', 'stark', 'theme', 'themes/engines/phptemplate/phptemplate.engine', 0, 0, -1, 0, 0x613a31363a7b733a343a226e616d65223b733a353a22537461726b223b733a31313a226465736372697074696f6e223b733a3230383a2254686973207468656d652064656d6f6e737472617465732044727570616c27732064656661756c742048544d4c206d61726b757020616e6420435353207374796c65732e20546f206c6561726e20686f7720746f206275696c6420796f7572206f776e207468656d6520616e64206f766572726964652044727570616c27732064656661756c7420636f64652c2073656520746865203c6120687265663d22687474703a2f2f64727570616c2e6f72672f7468656d652d6775696465223e5468656d696e672047756964653c2f613e2e223b733a373a227061636b616765223b733a343a22436f7265223b733a373a2276657273696f6e223b733a343a22372e3334223b733a343a22636f7265223b733a333a22372e78223b733a31313a227374796c65736865657473223b613a313a7b733a333a22616c6c223b613a313a7b733a31303a226c61796f75742e637373223b733a32333a227468656d65732f737461726b2f6c61796f75742e637373223b7d7d733a373a2270726f6a656374223b733a363a2264727570616c223b733a393a22646174657374616d70223b733a31303a2231343136343239343838223b733a363a22656e67696e65223b733a31313a2270687074656d706c617465223b733a373a22726567696f6e73223b613a393a7b733a31333a22736964656261725f6669727374223b733a31323a224c6566742073696465626172223b733a31343a22736964656261725f7365636f6e64223b733a31333a2252696768742073696465626172223b733a373a22636f6e74656e74223b733a373a22436f6e74656e74223b733a363a22686561646572223b733a363a22486561646572223b733a363a22666f6f746572223b733a363a22466f6f746572223b733a31313a22686967686c696768746564223b733a31313a22486967686c696768746564223b733a343a2268656c70223b733a343a2248656c70223b733a383a22706167655f746f70223b733a383a225061676520746f70223b733a31313a22706167655f626f74746f6d223b733a31313a225061676520626f74746f6d223b7d733a383a226665617475726573223b613a393a7b693a303b733a343a226c6f676f223b693a313b733a373a2266617669636f6e223b693a323b733a343a226e616d65223b693a333b733a363a22736c6f67616e223b693a343b733a31373a226e6f64655f757365725f70696374757265223b693a353b733a32303a22636f6d6d656e745f757365725f70696374757265223b693a363b733a32353a22636f6d6d656e745f757365725f766572696669636174696f6e223b693a373b733a393a226d61696e5f6d656e75223b693a383b733a31343a227365636f6e646172795f6d656e75223b7d733a31303a2273637265656e73686f74223b733a32373a227468656d65732f737461726b2f73637265656e73686f742e706e67223b733a333a22706870223b733a353a22352e322e34223b733a373a2273637269707473223b613a303a7b7d733a353a226d74696d65223b693a313431363432393438383b733a31343a22726567696f6e735f68696464656e223b613a323a7b693a303b733a383a22706167655f746f70223b693a313b733a31313a22706167655f626f74746f6d223b7d7d);

-- --------------------------------------------------------

--
-- Table structure for table `test`
--

CREATE TABLE `test` (
  `id` int(11) NOT NULL,
  `test` varchar(255) DEFAULT NULL COMMENT 'name of the current test',
  `type` varchar(255) NOT NULL COMMENT 'type of data stored here',
  `value` longblob DEFAULT NULL COMMENT 'the data'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='transient data while testing offline' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Stand-in structure for view `txs`
-- (See below for the actual view)
--
CREATE TABLE `txs` (
`xid` bigint(20)
,`actorId` bigint(20)
,`actorAgentId` bigint(20)
,`flags` bigint(20) unsigned
,`channel` tinyint(4)
,`boxId` bigint(20)
,`goods` tinyint(4)
,`risk` float
,`risks` bigint(20) unsigned
,`recursId` bigint(20)
,`reversesXid` bigint(20)
,`created` bigint(20)
,`type` tinyint(4)
,`amt` decimal(11,2)
,`rule` int(11)
,`relType` varchar(1)
,`rel` bigint(20)
,`eid` int(11)
,`for2` mediumtext
,`uid2` bigint(20)
,`agt2` bigint(20)
,`cat2` bigint(20)
,`for1` mediumtext
,`uid1` bigint(20)
,`agt1` bigint(20)
,`cat1` bigint(20)
);

-- --------------------------------------------------------

--
-- Table structure for table `txs2`
--

CREATE TABLE `txs2` (
  `txid` bigint(20) NOT NULL COMMENT 'the unique transaction ID',
  `amount` decimal(11,2) DEFAULT NULL COMMENT 'amount of transfer',
  `payee` bigint(20) DEFAULT NULL COMMENT 'CG account record ID',
  `created` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime transaction was created',
  `completed` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime transaction was completed',
  `deposit` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime transfer check was printed and deposited',
  `bankAccount` blob DEFAULT NULL COMMENT 'Bank account for the transfer',
  `isSavings` tinyint(4) DEFAULT NULL COMMENT '1 if bankAccount is a savings account',
  `risk` float DEFAULT NULL COMMENT 'suspiciousness rating',
  `risks` bigint(20) UNSIGNED NOT NULL DEFAULT 0 COMMENT 'list of risk factors',
  `bankTxId` bigint(20) DEFAULT NULL COMMENT 'bank transaction ID',
  `channel` tinyint(4) DEFAULT NULL COMMENT 'through what medium was the transaction entered',
  `xid` bigint(20) NOT NULL DEFAULT 0 COMMENT 'id of related tx_hdrs record',
  `pid` bigint(20) DEFAULT NULL COMMENT 'related people record ID'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Record of bank transfers in the region' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Stand-in structure for view `txs2_bank`
-- (See below for the actual view)
--
CREATE TABLE `txs2_bank` (
`txid` bigint(20)
,`amount` decimal(11,2)
,`payee` bigint(20)
,`created` int(11)
,`completed` int(11)
,`deposit` int(11)
,`bankAccount` blob
,`isSavings` tinyint(4)
,`risk` float
,`risks` bigint(20) unsigned
,`bankTxId` bigint(20)
,`channel` tinyint(4)
,`xid` bigint(20)
,`pid` bigint(20)
);

-- --------------------------------------------------------

--
-- Stand-in structure for view `txs2_outer`
-- (See below for the actual view)
--
CREATE TABLE `txs2_outer` (
`txid` bigint(20)
,`amount` decimal(11,2)
,`payee` bigint(20)
,`created` int(11)
,`completed` int(11)
,`deposit` int(11)
,`bankAccount` blob
,`isSavings` tinyint(4)
,`risk` float
,`risks` bigint(20) unsigned
,`bankTxId` bigint(20)
,`channel` tinyint(4)
,`xid` bigint(20)
,`pid` bigint(20)
);

-- --------------------------------------------------------

--
-- Stand-in structure for view `txs_all`
-- (See below for the actual view)
--
CREATE TABLE `txs_all` (
`xid` bigint(20)
,`actorId` bigint(20)
,`actorAgentId` bigint(20)
,`flags` bigint(20) unsigned
,`channel` tinyint(4)
,`boxId` bigint(20)
,`goods` tinyint(4)
,`risk` float
,`risks` bigint(20) unsigned
,`recursId` bigint(20)
,`reversesXid` bigint(20)
,`created` bigint(20)
,`type` tinyint(4)
,`amt` decimal(11,2)
,`rule` int(11)
,`relType` varchar(1)
,`rel` bigint(20)
,`eid` int(11)
,`for2` mediumtext
,`uid2` bigint(20)
,`agt2` bigint(20)
,`cat2` bigint(20)
,`for1` mediumtext
,`uid1` bigint(20)
,`agt1` bigint(20)
,`cat1` bigint(20)
);

-- --------------------------------------------------------

--
-- Stand-in structure for view `txs_aux`
-- (See below for the actual view)
--
CREATE TABLE `txs_aux` (
`xid` bigint(20)
,`actorId` bigint(20)
,`actorAgentId` bigint(20)
,`flags` bigint(20) unsigned
,`channel` tinyint(4)
,`boxId` bigint(20)
,`goods` tinyint(4)
,`risk` float
,`risks` bigint(20) unsigned
,`recursId` bigint(20)
,`reversesXid` bigint(20)
,`created` bigint(20)
,`type` tinyint(4)
,`amt` decimal(11,2)
,`rule` int(11)
,`relType` varchar(1)
,`rel` bigint(20)
,`eid` int(11)
,`for2` mediumtext
,`uid2` bigint(20)
,`agt2` bigint(20)
,`cat2` bigint(20)
,`for1` mediumtext
,`uid1` bigint(20)
,`agt1` bigint(20)
,`cat1` bigint(20)
);

-- --------------------------------------------------------

--
-- Stand-in structure for view `txs_bank`
-- (See below for the actual view)
--
CREATE TABLE `txs_bank` (
`xid` bigint(20)
,`actorId` bigint(20)
,`actorAgentId` bigint(20)
,`flags` bigint(20) unsigned
,`channel` tinyint(4)
,`boxId` bigint(20)
,`goods` tinyint(4)
,`risk` float
,`risks` bigint(20) unsigned
,`recursId` bigint(20)
,`reversesXid` bigint(20)
,`created` bigint(20)
,`type` tinyint(4)
,`amt` decimal(11,2)
,`rule` int(11)
,`relType` varchar(1)
,`rel` bigint(20)
,`eid` int(11)
,`for2` mediumtext
,`uid2` bigint(20)
,`agt2` bigint(20)
,`cat2` bigint(20)
,`for1` mediumtext
,`uid1` bigint(20)
,`agt1` bigint(20)
,`cat1` bigint(20)
);

-- --------------------------------------------------------

--
-- Stand-in structure for view `txs_bank_only`
-- (See below for the actual view)
--
CREATE TABLE `txs_bank_only` (
`xid` bigint(20)
,`actorId` bigint(20)
,`actorAgentId` bigint(20)
,`flags` bigint(20) unsigned
,`channel` tinyint(4)
,`boxId` bigint(20)
,`goods` tinyint(4)
,`risk` float
,`risks` bigint(20) unsigned
,`recursId` bigint(20)
,`reversesXid` bigint(20)
,`created` bigint(20)
,`type` tinyint(4)
,`amt` decimal(11,2)
,`rule` int(11)
,`relType` varchar(1)
,`rel` bigint(20)
,`eid` int(11)
,`for2` mediumtext
,`uid2` bigint(20)
,`agt2` bigint(20)
,`cat2` bigint(20)
,`for1` mediumtext
,`uid1` bigint(20)
,`agt1` bigint(20)
,`cat1` bigint(20)
);

-- --------------------------------------------------------

--
-- Stand-in structure for view `txs_ever`
-- (See below for the actual view)
--
CREATE TABLE `txs_ever` (
`xid` bigint(20)
,`actorId` bigint(20)
,`actorAgentId` bigint(20)
,`flags` bigint(20) unsigned
,`channel` tinyint(4)
,`boxId` bigint(20)
,`goods` tinyint(4)
,`risk` float
,`risks` bigint(20) unsigned
,`recursId` bigint(20)
,`reversesXid` bigint(20)
,`created` bigint(20)
,`type` tinyint(4)
,`amt` decimal(11,2)
,`rule` int(11)
,`relType` varchar(1)
,`rel` bigint(20)
,`eid` int(11)
,`for2` mediumtext
,`uid2` bigint(20)
,`agt2` bigint(20)
,`cat2` bigint(20)
,`for1` mediumtext
,`uid1` bigint(20)
,`agt1` bigint(20)
,`cat1` bigint(20)
);

-- --------------------------------------------------------

--
-- Stand-in structure for view `txs_noreverse`
-- (See below for the actual view)
--
CREATE TABLE `txs_noreverse` (
`xid` bigint(20)
,`actorId` bigint(20)
,`actorAgentId` bigint(20)
,`flags` bigint(20) unsigned
,`channel` tinyint(4)
,`boxId` bigint(20)
,`goods` tinyint(4)
,`risk` float
,`risks` bigint(20) unsigned
,`recursId` bigint(20)
,`reversesXid` bigint(20)
,`created` bigint(20)
,`type` tinyint(4)
,`amt` decimal(11,2)
,`rule` int(11)
,`relType` varchar(1)
,`rel` bigint(20)
,`eid` int(11)
,`for2` mediumtext
,`uid2` bigint(20)
,`agt2` bigint(20)
,`cat2` bigint(20)
,`for1` mediumtext
,`uid1` bigint(20)
,`agt1` bigint(20)
,`cat1` bigint(20)
);

-- --------------------------------------------------------

--
-- Stand-in structure for view `txs_outer`
-- (See below for the actual view)
--
CREATE TABLE `txs_outer` (
`xid` bigint(20)
,`actorId` bigint(20)
,`actorAgentId` bigint(20)
,`flags` bigint(20) unsigned
,`channel` tinyint(4)
,`boxId` bigint(20)
,`goods` tinyint(4)
,`risk` float
,`risks` bigint(20) unsigned
,`recursId` bigint(20)
,`reversesXid` bigint(20)
,`created` bigint(20)
,`type` tinyint(4)
,`amt` decimal(11,2)
,`rule` int(11)
,`relType` varchar(1)
,`rel` bigint(20)
,`eid` int(11)
,`for2` mediumtext
,`uid2` bigint(20)
,`agt2` bigint(20)
,`cat2` bigint(20)
,`for1` mediumtext
,`uid1` bigint(20)
,`agt1` bigint(20)
,`cat1` bigint(20)
);

-- --------------------------------------------------------

--
-- Stand-in structure for view `txs_prime`
-- (See below for the actual view)
--
CREATE TABLE `txs_prime` (
`xid` bigint(20)
,`actorId` bigint(20)
,`actorAgentId` bigint(20)
,`flags` bigint(20) unsigned
,`channel` tinyint(4)
,`boxId` bigint(20)
,`goods` tinyint(4)
,`risk` float
,`risks` bigint(20) unsigned
,`recursId` bigint(20)
,`reversesXid` bigint(20)
,`created` bigint(20)
,`type` tinyint(4)
,`amt` decimal(11,2)
,`rule` int(11)
,`relType` varchar(1)
,`rel` bigint(20)
,`eid` int(11)
,`for2` mediumtext
,`uid2` bigint(20)
,`agt2` bigint(20)
,`cat2` bigint(20)
,`for1` mediumtext
,`uid1` bigint(20)
,`agt1` bigint(20)
,`cat1` bigint(20)
);

-- --------------------------------------------------------

--
-- Stand-in structure for view `txs_proper`
-- (See below for the actual view)
--
CREATE TABLE `txs_proper` (
`xid` bigint(20)
,`actorId` bigint(20)
,`actorAgentId` bigint(20)
,`flags` bigint(20) unsigned
,`channel` tinyint(4)
,`boxId` bigint(20)
,`goods` tinyint(4)
,`risk` float
,`risks` bigint(20) unsigned
,`recursId` bigint(20)
,`reversesXid` bigint(20)
,`created` bigint(20)
,`type` tinyint(4)
,`amt` decimal(11,2)
,`rule` int(11)
,`relType` varchar(1)
,`rel` bigint(20)
,`eid` int(11)
,`for2` mediumtext
,`uid2` bigint(20)
,`agt2` bigint(20)
,`cat2` bigint(20)
,`for1` mediumtext
,`uid1` bigint(20)
,`agt1` bigint(20)
,`cat1` bigint(20)
);

-- --------------------------------------------------------

--
-- Stand-in structure for view `txs_rebate`
-- (See below for the actual view)
--
CREATE TABLE `txs_rebate` (
`xid` bigint(20)
,`actorId` bigint(20)
,`actorAgentId` bigint(20)
,`flags` bigint(20) unsigned
,`channel` tinyint(4)
,`boxId` bigint(20)
,`goods` tinyint(4)
,`risk` float
,`risks` bigint(20) unsigned
,`recursId` bigint(20)
,`reversesXid` bigint(20)
,`created` bigint(20)
,`type` tinyint(4)
,`amt` decimal(11,2)
,`rule` int(11)
,`relType` varchar(1)
,`rel` bigint(20)
,`eid` int(11)
,`for2` mediumtext
,`uid2` bigint(20)
,`agt2` bigint(20)
,`cat2` bigint(20)
,`for1` mediumtext
,`uid1` bigint(20)
,`agt1` bigint(20)
,`cat1` bigint(20)
);

-- --------------------------------------------------------

--
-- Stand-in structure for view `txs_usd_fee`
-- (See below for the actual view)
--
CREATE TABLE `txs_usd_fee` (
`xid` bigint(20)
,`actorId` bigint(20)
,`actorAgentId` bigint(20)
,`flags` bigint(20) unsigned
,`channel` tinyint(4)
,`boxId` bigint(20)
,`goods` tinyint(4)
,`risk` float
,`risks` bigint(20) unsigned
,`recursId` bigint(20)
,`reversesXid` bigint(20)
,`created` bigint(20)
,`type` tinyint(4)
,`amt` decimal(11,2)
,`rule` int(11)
,`relType` varchar(1)
,`rel` bigint(20)
,`eid` int(11)
,`for2` mediumtext
,`uid2` bigint(20)
,`agt2` bigint(20)
,`cat2` bigint(20)
,`for1` mediumtext
,`uid1` bigint(20)
,`agt1` bigint(20)
,`cat1` bigint(20)
);

-- --------------------------------------------------------

--
-- Table structure for table `tx_bads`
--

CREATE TABLE `tx_bads` (
  `id` bigint(20) NOT NULL COMMENT 'record ID',
  `version` mediumint(8) NOT NULL COMMENT 'app version number',
  `deviceId` mediumtext DEFAULT NULL COMMENT 'ID of the device submitting the transaction',
  `actorId` varchar(255) DEFAULT NULL COMMENT 'account ID of the transaction initiator',
  `otherId` varchar(255) DEFAULT NULL COMMENT 'other account ID',
  `amount` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'amount to pay or charge',
  `description` mediumtext DEFAULT NULL COMMENT 'description of transactions',
  `created` bigint(20) DEFAULT NULL COMMENT 'date/time of transaction',
  `proof` varchar(255) DEFAULT NULL COMMENT 'various parameters hashed together with cardCode',
  `offline` tinyint(4) NOT NULL COMMENT 'transaction taken online (0) or offline (1)',
  `problem` mediumtext DEFAULT NULL COMMENT 'problem description'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_general_ci COMMENT='transactions that cannot be completed, requested by our app';

-- --------------------------------------------------------

--
-- Table structure for table `tx_cats`
--

CREATE TABLE `tx_cats` (
  `id` int(11) NOT NULL,
  `category` varchar(255) DEFAULT NULL COMMENT 'category',
  `description` longtext DEFAULT NULL COMMENT 'description of category',
  `externalId` mediumint(8) DEFAULT NULL COMMENT 'account record ID in external accounting program',
  `show` tinyint(4) NOT NULL DEFAULT 1 COMMENT 'whether to show this category in dropdowns',
  `line990` varchar(255) DEFAULT NULL COMMENT 'section, part, and line number of this category on IRS Form 990',
  `nick` varchar(255) DEFAULT NULL COMMENT 'nickname for the account, used to set transaction category'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_general_ci COMMENT='income and expense categories';

--
-- Dumping data for table `tx_cats`
--

INSERT INTO `tx_cats` VALUES(-1, 'Loan or CG-to-CG transfer', 'Loan to or from a CG Loan Fund OR a CG-to-CG transfer (internal only)', 0, 1, NULL, 'CG2CG');
INSERT INTO `tx_cats` VALUES(100, 'I: Billable Expense Income', '', 265, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(200, 'I: Consulting and Services', 'Consulting Income', 155, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(300, 'I: Donations', '', 156, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(400, 'I: Donations: Ads', '', 157, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(500, 'I: Donations: Company Donations', '', 233, 1, NULL, 'D-COMPANY');
INSERT INTO `tx_cats` VALUES(600, 'I: Donations: Crumbs Donations', 'percentage of payments received through Common Good', 280, 1, NULL, 'D-CRUMB');
INSERT INTO `tx_cats` VALUES(800, 'I: Donations: Grants', '', 159, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(900, 'I: Donations: One-time Donations', 'usually from non-members (always, before FY2019)', 234, 1, NULL, 'D-ONCE');
INSERT INTO `tx_cats` VALUES(1000, 'I: Donations: Regular Donations', 'recurring monthly, quarterly, or yearly (until FY2019 included ALL member donations)', 196, 1, NULL, 'D-REGULAR');
INSERT INTO `tx_cats` VALUES(1100, 'I: Donations: Roundup Donations', 'automatic contribution of rounded up payment change', 279, 1, NULL, 'D-ROUNDUP');
INSERT INTO `tx_cats` VALUES(1150, 'I: Donations: Sponsored', 'donations to sponsored projects', 194, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(1200, 'I: Donations: Sponsored: Sponsored Donations', 'passed through to an organization we fiscally sponsor', 318, 1, NULL, 'D-FBO');
INSERT INTO `tx_cats` VALUES(1250, 'I: Donations: Sponsored: Sponsored Stepups', 'stepup donations to sponsored projects', 315, 1, NULL, 'D-FBO-STEPUP');
INSERT INTO `tx_cats` VALUES(1300, 'I: Donations: Sponsored Donations: Fiscal Sponsorship Fees', 'part of Sponsored Donations not to be spent on program costs', 322, 1, NULL, 'FS-FEE');
INSERT INTO `tx_cats` VALUES(1400, 'I: Donations: Stepup Donations and Tips', '', 311, 1, NULL, 'D-STEPUP');
INSERT INTO `tx_cats` VALUES(1700, 'I: Gross Sales', 'Gross Sales', 161, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(1800, 'I: Income Uncertainty', 'Rewards, Inflation Adjustment, and Shared Rewards', 222, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(1900, 'I: Investment Income', '', 295, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(2000, 'I: Investment Income: Bank Interest', 'Bank Interest', 244, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(2100, 'I: Investment Income: Program-related Investment Income', 'Investment Income in keeping with our mission', 162, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(2200, 'I: Markup', '', 258, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(2300, 'I: Sales of Product Income', '', 262, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(2400, 'I: Uncategorized Income', 'Income not categorized elsewh', 188, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(2500, 'E: Ask William', '', 267, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(2600, 'E: Bad Debt', 'Bad Debt Expense', 164, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(2700, 'E: Change in Risk Assessment', 'Change in expected value of investments or contingent grants (in or out)', 293, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(2800, 'E: Cost of Goods Sold', '', 263, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(2900, 'E: Depreciation', '', 281, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(3000, 'E: Depreciation: Depreciation - FY2014 Furniture', '', 286, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(3100, 'E: Depreciation: Depreciation - FY2016 Equipment', '', 283, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(3200, 'E: Depreciation: Depreciation - FY2017 Equipment', '', 284, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(3300, 'E: Depreciation: Depreciation - FY2018 Equipment', '', 285, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(3400, 'E: Depreciation: Depreciation FY2014 - Equipment', '', 282, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(3500, 'E: Equipment', '', 165, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(3600, 'E: Equipment: CG POS equipment', '', 202, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(3700, 'E: Equipment: Computer Hardware & Software', '', 190, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(3800, 'E: Equipment: Repairs', 'Repairs', 182, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(3900, 'E: Equipment: Resources (Books, etc.)', '', 183, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(4000, 'E: Event Costs', 'Food, lodging, equipment etc.', 166, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(4100, 'E: Event Costs: Consumables', '', 207, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(4200, 'E: Event Costs: Equipment Rental', '', 246, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(4300, 'E: Event Costs: Event Fees & Space Rental', '', 169, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(4400, 'E: Fees', '', 167, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(4500, 'E: Fees: Government Fees', '', 170, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(4600, 'E: Fees: Interest Expense', 'Interest Expense', 174, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(4700, 'E: Fees: Legal Fees', '', 171, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(4800, 'E: Fees: Tax Penalties and Interest', '', 240, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(4900, 'E: Fees: Transaction Fees', 'fees for credit card transactions, wires, etc.', 168, 1, NULL, 'TX-FEE');
INSERT INTO `tx_cats` VALUES(5000, 'E: Fees: Transaction Fees: Reimbursement of Transaction Fees', 'mostly from sponsored organizations', 323, 1, NULL, 'TX-FEE-BACK');
INSERT INTO `tx_cats` VALUES(5100, 'E: Grants', '', 172, 1, NULL, 'TO-ORG');
INSERT INTO `tx_cats` VALUES(5200, 'E: Information Services', 'Dues and Subscription Expense', 173, 1, NULL, 'INFO-SVC');
INSERT INTO `tx_cats` VALUES(5300, 'E: Information Services: Websites', '', 185, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(5400, 'E: Marketing - Advanced', 'Promotional Expenses', 198, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(5500, 'E: Marketing - Advanced: Entertainment', 'Entertainment', 256, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(5600, 'E: Marketing - Advanced: Networking Fees and Dues', '', 253, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(5700, 'E: Marketing - Simple', '', 230, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(5800, 'E: Marketing - Simple: Advertising', 'Advertising', 251, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(5900, 'E: Marketing - Simple: Member Support', '', 303, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(6000, 'E: Marketing - Simple: Postage / Shipping', 'Postage and Delivery Expense', 255, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(6100, 'E: Marketing - Simple: Printing', 'Printing and Repro. Expense', 248, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(6200, 'E: Marketing - Simple: Promotional Materials', '', 245, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(6300, 'E: Miscellaneous', 'Miscellaneous', 175, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(6400, 'E: Miscellaneous: CG Account Adjustments', '', 203, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(6500, 'E: Miscellaneous: Reconciliation Discrepancies', 'Discrepancies between bank statements and company records', 226, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(6600, 'E: Office', '', 176, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(6700, 'E: Office: Communication Services', 'internet, phone, zoom, otter.ai, etc.', 242, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(6800, 'E: Office: Insurance', 'Insurance', 252, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(6900, 'E: Office: Rent', '', 189, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(7000, 'E: Office: Supplies', 'Supplies', 177, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(7100, 'E: Office: Utilities', 'Water, Gas, Electric', 178, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(7200, 'E: Opening Balance Equity', 'Opening balances during setup', 145, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(7300, 'E: Payroll Expenses', 'Payroll expenses', 209, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(7400, 'E: Payroll Expenses: Payroll Fee', '', 232, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(7500, 'E: Payroll Expenses: Payroll Taxes', '', 237, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(7600, 'E: Payroll Expenses: Payroll Wages', '', 239, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(7700, 'E: Professional Fees', '', 179, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(7800, 'E: Professional Fees: Accounting Services', '', 201, 1, NULL, 'ACCOUNTING');
INSERT INTO `tx_cats` VALUES(7900, 'E: Professional Fees: Ad Hoc Consultants', '', 180, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(8000, 'E: Professional Fees: Computer Services', '', 220, 1, NULL, 'COMPUTER');
INSERT INTO `tx_cats` VALUES(8100, 'E: Professional Fees: Honoraria', 'appreciative discretionary compensation for miscellaneous services -- includes \"thank you\" gifts to', 219, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(8200, 'E: Professional Fees: Staff Consultants', 'Professional Fees', 181, 1, NULL, 'CONTRACTOR');
INSERT INTO `tx_cats` VALUES(8300, 'E: Professional Fees: Staff Development', '', 224, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(8400, 'E: Purchases', '', 261, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(8500, 'E: Reconciliation Discrepancies', '', 266, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(8600, 'E: Reconciliation Discrepancies-1', '', 296, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(8700, 'E: Retained Earnings', 'Undistributed earnings of the', 144, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(8800, 'E: Sponsored Project Expenses', '', 314, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(8810, 'E: Sponsored Project Expenses: Advertising and Promotion', '', 347, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(8820, 'E: Sponsored Project Expenses: Conferences, Conventions, and Meetings', '', 352, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(8830, 'E: Sponsored Project Expenses: Fees for Services', '', 340, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(8840, 'E: Sponsored Project Expenses: Fees for Services: Accounting', '', 343, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(8850, 'E: Sponsored Project Expenses: Fees for Services: Fundraising', '', 344, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(8860, 'E: Sponsored Project Expenses: Fees for Services: Investment Management', '', 345, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(8870, 'E: Sponsored Project Expenses: Fees for Services: Legal', '', 341, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(8880, 'E: Sponsored Project Expenses: Fees for Services: Management', '', 342, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(8890, 'E: Sponsored Project Expenses: Fees for Services: Staff Development', '', 346, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(8895, 'E: Sponsored Project Expenses: Government Fees', '', 355, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(8900, 'E: Sponsored Project Expenses: Grants and Direct Support to Individuals', 'Direct support to eligible members for food costs', 316, 1, NULL, 'TO-PERSON');
INSERT INTO `tx_cats` VALUES(8905, 'E: Sponsored Project Expenses: Grants to Domestic Organizations', '', 338, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(8910, 'E: Sponsored Project Expenses: Grants to Foreign Organizations and Individuals', '', 339, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(8920, 'E: Sponsored Project Expenses: Information Technology', '', 349, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(8930, 'E: Sponsored Project Expenses: Insurance', '', 354, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(8940, 'E: Sponsored Project Expenses: Interest Expense', '', 353, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(8945, 'E: Sponsored Project Expenses: Office Expenses', '', 348, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(8950, 'E: Sponsored Project Expenses: Project Equipment', 'equipment used for a sponsored project', 337, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(9000, 'E: Sponsored Project Expenses: Real Estate Purchases and Occupancy', 'rent or acquisition of real estate', 325, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(9050, 'E: Sponsored Project Expenses: Royalties', '', 350, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(9100, 'E: Sponsored Project Expenses: Payroll', '', 317, 1, NULL, 'FBO-LABOR');
INSERT INTO `tx_cats` VALUES(9125, 'E: Sponsored Project Expenses: Payroll Taxes', '', 356, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(9150, 'E: Sponsored Project Expenses: Transaction Fees', 'fees for credit card transactions, wires, etc.', 328, 1, NULL, 'FBO-TX-FEE');
INSERT INTO `tx_cats` VALUES(9175, 'E: Sponsored Project Expenses: Travel', '', 351, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(9200, 'E: Travel', 'Car & Truck', 184, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(9300, 'E: Unapplied Cash Bill Payment Expense', '', 297, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(9400, 'E: Uncategorized Expense', '', 259, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(9500, 'E: _Accrued Int', 'Accrued Interest', 186, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(9600, 'E: _IntExp', 'Investment Interest Exp', 187, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(9700, 'A: *Accounts Receivable', 'Unpaid or unapplied customer', 193, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(9800, 'A: 457 Escrow Asset', '', 290, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(9900, 'A: 457 Escrow Asset: 457 Investments', 'money is in shared capital cooperative', 302, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(10000, 'A: Accounts Receivable', '', 152, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(10100, 'A: CC Processor', 'credit card processor, like PayPal', 150, 1, NULL, 'PROCESSOR');
INSERT INTO `tx_cats` VALUES(10150, 'A: CG 457 Escrow', '', 223, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(10200, 'A: CG Account ..AAB', 'Common Good\'s own Common Good Credits Account', 195, 1, NULL, 'NEWAAB');
INSERT INTO `tx_cats` VALUES(10300, 'A: Inventory Asset', '', 264, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(10400, 'A: Investments', '', 211, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(10500, 'A: Investments: Artisan Beverage Coop Investment', '', 277, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(10600, 'A: Investments: Boston Community Loan Fund', '', 273, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(10700, 'A: Investments: CG Western MA Region Investments', 'investments / loans made by the \"Region\"', 312, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(10800, 'A: Investments: Co-op Power Investment', '', 278, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(10900, 'A: Investments: Equity Trust Investment', '', 274, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(11000, 'A: Investments: NH Community Loan Fund', '', 269, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(11100, 'A: Investments: Northeast Biodiesel Investment', '', 268, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(11150, 'A: Investments: Other Loans', '', 329, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(11200, 'A: Investments: PVGrows Investment', '', 275, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(11300, 'A: Investments: x Loan Loss Reserve - CG Western MA', '', 313, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(11400, 'A: Investments: x Loan Loss Reserve - Community Funds (5%)', '', 288, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(11500, 'A: Investments: x Loan Loss Reserve - Northeast Biodiesel (50%)', '', 287, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(11600, 'A: Office Equipment', 'major equipment such as computers (depreciate at 20% per year)', 214, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(11700, 'A: Office Furniture', '', 212, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(11800, 'A: Old Checking ..2275', 'at Citizens Bank. Old corporate account #909302151 at Greenfield Cooperative Bank. Closed October 31', 146, 1, NULL, 'OLD-BANK');
INSERT INTO `tx_cats` VALUES(11900, 'A: Operations ..8571', 'Brattleboro S&L account #500718571', 307, 1, NULL, 'OPERATIONS');
INSERT INTO `tx_cats` VALUES(12000, 'A: Petty Cash', '', 151, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(12100, 'A: Sponsored', 'Projects sponsored by CG', 319, 1, NULL, 'SPONSORED');
INSERT INTO `tx_cats` VALUES(12200, 'A: Sponsored: CG Western MA Region ..AAA', 'The server\'s \"regional\" account', 310, 1, NULL, '!NEWAAA');
INSERT INTO `tx_cats` VALUES(12250, 'A: Sponsored: Dollar Pool', '', 330, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(12275, 'A: Sponsored: Dollar Pool: CC Processor Sponsored', 'part of CC account that is part of the Dollar Pool', 205, 1, NULL, 'FBO-PROCESSOR');
INSERT INTO `tx_cats` VALUES(12300, 'A: Sponsored: Dollar Pool ..8598', 'Brattleboro S&L account #500718598', 306, 1, NULL, 'AAAAJV');
INSERT INTO `tx_cats` VALUES(12400, 'A: Sponsored: Dollar Pool ..8598: MSB Escrow for Dollar Pool', 'BS&L asked to hold $50k in escrow', 327, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(12500, 'A: Sponsored: Food Fund ..AZV', 'Common Good\'s fund for food subsidies', 309, 1, NULL, 'NEWAZV');
INSERT INTO `tx_cats` VALUES(12600, 'A: Sponsored: Kibilio ..BTY', 'FBO Kibilio (fiscal sponsorship)', 320, 1, NULL, 'NEWBTY');
INSERT INTO `tx_cats` VALUES(12700, 'A: Sponsored: RJ Brooklyn ..AUN', 'FBO Racial Justice Brooklyn (fiscal sponsorship)', 321, 1, NULL, 'NYAAUN');
INSERT INTO `tx_cats` VALUES(12800, 'A: Uncategorized Asset', '', 260, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(12900, 'L: 457 Deferred Retirement Pay', '', 299, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(13000, 'L: 457 Deferred Retirement Pay: 457 Deferred Retirement Pay - WS', '', 218, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(13100, 'L: Accounts Payable', 'Unpaid or unapplied vendor bills or credits', 231, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(13200, 'L: Capital One CC', '', 153, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(13300, 'L: Community Funding Guarantees FY2017-18 (10%)', 'We promised Common Good Greenfield, to back their first two year\'s funding ($10 + $18k).', 289, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(13400, 'L: Contingent Compensation', 'compensation CGF owes to some former contractors', 192, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(13500, 'L: Dollar Pool Liability', 'Amt in CG Dollar Pool we owe to CG Communities (their members, collectively) -- net member transfers', 204, 1, NULL, 'POOL');
INSERT INTO `tx_cats` VALUES(13600, 'L: EIDL Advance', '', 305, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(13700, 'L: Equipment Deposits', '', 225, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(13800, 'L: Long Term Loans', '', 229, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(13900, 'L: Long Term Loans: Advance Investments', 'for CGBank Project', 243, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(14000, 'L: Long Term Loans: CG Greenfield Loan for Co-op Power', '', 292, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(14100, 'L: Long Term Loans: Forgivable Loans', '', 254, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(14150, 'L: Long Term Loans: Loan Fund Liability', 'money lent to Common Good loan funds, still owed back', 374, 1, NULL, 'LOANFUND');
INSERT INTO `tx_cats` VALUES(14200, 'L: Long Term Loans: Loan from Sally Willoughby', 'at 2.5% ($250 due 2/1/2020, $10,250 due 2/1/2021)', 300, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(14300, 'L: Long Term Loans: S2BE checking accounts', 'money owed to participants', 250, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(14400, 'L: Negative Balance Risk of Non-Repayment (10%)', 'How much of the total of all negative balances would people fail to repay.', 308, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(14500, 'L: Payroll Liabilities', 'Unpaid payroll liabilities. Amounts withheld or accrued, but not yet paid', 210, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(14600, 'L: Payroll Liabilities: 941 Taxes Liability', '', 236, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(14700, 'L: Payroll Liabilities: State SUTA/UI Tax Liability', '', 238, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(14800, 'L: Payroll Liabilities: State Withholding Tax Liability', '', 217, 1, NULL, NULL);
INSERT INTO `tx_cats` VALUES(14900, 'L: Payroll Liabilities: To Be Paid In CG Credits', '', 215, 1, NULL, 'LABOR');

-- --------------------------------------------------------

--
-- Stand-in structure for view `tx_credits`
-- (See below for the actual view)
--
CREATE TABLE `tx_credits` (
`id` bigint(20)
,`created` int(11)
,`fromUid` bigint(20)
,`toUid` bigint(20)
,`amount` decimal(11,2)
,`xid` int(11)
,`purpose` longtext
);

-- --------------------------------------------------------

--
-- Stand-in structure for view `tx_disputes`
-- (See below for the actual view)
--
CREATE TABLE `tx_disputes` (
`id` int(11)
,`xid` bigint(20)
,`uid` bigint(20)
,`agentUid` bigint(20)
,`reason` varchar(255)
,`status` tinyint(4)
,`deleted` bigint(20)
);

-- --------------------------------------------------------

--
-- Table structure for table `tx_disputes_all`
--

CREATE TABLE `tx_disputes_all` (
  `id` int(11) NOT NULL,
  `xid` bigint(20) NOT NULL COMMENT 'id of the transaction in dispute',
  `uid` bigint(20) NOT NULL COMMENT 'id of the user who disputes the transaction',
  `agentUid` bigint(20) NOT NULL COMMENT 'id of the user who actually acted for the nominal user',
  `reason` varchar(255) NOT NULL COMMENT 'reason the transaction is being disputes',
  `status` tinyint(4) NOT NULL DEFAULT 1 COMMENT 'status of the dispute',
  `deleted` bigint(20) DEFAULT NULL COMMENT 'unix timestamp of when the dispute record was deleted, null if it hasn''t been'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='record of dispute of transaction' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Stand-in structure for view `tx_entries`
-- (See below for the actual view)
--
CREATE TABLE `tx_entries` (
`id` int(11)
,`xid` bigint(20)
,`entryType` tinyint(4)
,`amount` decimal(11,2)
,`uid` bigint(20)
,`agentUid` bigint(20)
,`description` mediumtext
,`cat` bigint(20)
,`relType` varchar(1)
,`relatedId` bigint(20)
,`rule` int(11)
);

-- --------------------------------------------------------

--
-- Table structure for table `tx_entries_all`
--

CREATE TABLE `tx_entries_all` (
  `id` int(11) NOT NULL,
  `xid` bigint(20) NOT NULL DEFAULT 0 COMMENT 'the ID of the transaction to which this entry belongs',
  `entryType` tinyint(4) DEFAULT NULL COMMENT 'entry type',
  `amount` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'amount, may be negative',
  `uid` bigint(20) NOT NULL COMMENT 'user id of the account to which this entry applies',
  `agentUid` bigint(20) DEFAULT NULL COMMENT 'user id of account''s agent (who approved this transaction for this account)',
  `description` mediumtext NOT NULL DEFAULT 'NULL' COMMENT 'description for this entry',
  `cat` bigint(20) DEFAULT NULL COMMENT 'related tx_cats record ID',
  `relType` varchar(1) DEFAULT NULL COMMENT 'type of related record, ''D'' for coupated, ''I'' for invoice',
  `relatedId` bigint(20) DEFAULT NULL COMMENT 'id of related record',
  `deleted` bigint(20) DEFAULT NULL COMMENT 'UNIXTIME when record was deleted, else null',
  `rule` int(11) DEFAULT NULL COMMENT 'Auxiliary transaction to which this entry is related.'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Record of a transaction line entry' ROW_FORMAT=DYNAMIC;

--
-- Triggers `tx_entries_all`
--
DELIMITER $$
CREATE TRIGGER `delEntry` AFTER DELETE ON `tx_entries_all` FOR EACH ROW UPDATE users u SET balance=balance-OLD.amount
    WHERE OLD.uid IN (u.uid, u.jid)
$$
DELIMITER ;
DELIMITER $$
CREATE TRIGGER `insEntry` AFTER INSERT ON `tx_entries_all` FOR EACH ROW UPDATE users u SET balance=balance+NEW.amount
    WHERE NEW.uid IN (u.uid, u.jid)
$$
DELIMITER ;
DELIMITER $$
CREATE TRIGGER `updEntry` AFTER UPDATE ON `tx_entries_all` FOR EACH ROW UPDATE users u SET balance=balance
      -IF(OLD.uid IN (u.uid, u.jid), IF(OLD.deleted,0,OLD.amount), 0)
      +IF(NEW.uid IN (u.uid, u.jid), IF(NEW.deleted,0,NEW.amount), 0)
    WHERE OLD.uid IN (u.uid, u.jid) OR NEW.uid IN (u.uid, u.jid)
$$
DELIMITER ;

-- --------------------------------------------------------

--
-- Stand-in structure for view `tx_entries_o`
-- (See below for the actual view)
--
CREATE TABLE `tx_entries_o` (
`id` int(11)
,`xid` bigint(20)
,`entryType` tinyint(4)
,`amount` decimal(11,2)
,`uid` bigint(20)
,`agentUid` bigint(20)
,`description` mediumtext
,`cat` bigint(20)
,`relType` varchar(1)
,`relatedId` bigint(20)
,`rule` int(11)
);

-- --------------------------------------------------------

--
-- Stand-in structure for view `tx_hdrs`
-- (See below for the actual view)
--
CREATE TABLE `tx_hdrs` (
`xid` bigint(20)
,`actorId` bigint(20)
,`actorAgentId` bigint(20)
,`flags` bigint(20) unsigned
,`channel` tinyint(4)
,`boxId` bigint(20)
,`goods` tinyint(4)
,`risk` float
,`risks` bigint(20) unsigned
,`recursId` bigint(20)
,`reversesXid` bigint(20)
,`created` bigint(20)
);

-- --------------------------------------------------------

--
-- Table structure for table `tx_hdrs_all`
--

CREATE TABLE `tx_hdrs_all` (
  `xid` bigint(20) NOT NULL COMMENT 'the unique transaction ID',
  `actorId` bigint(20) NOT NULL COMMENT 'user id of the transaction''s initiator',
  `actorAgentId` bigint(20) DEFAULT NULL COMMENT 'user id of the agent for the initiator (who actually initiated this transaction)',
  `flags` bigint(20) UNSIGNED NOT NULL DEFAULT 0 COMMENT 'boolean characteristics and state flags',
  `channel` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'through what medium was the transaction entered',
  `boxId` bigint(20) DEFAULT NULL COMMENT 'on what machine was the transaction entered',
  `goods` tinyint(4) NOT NULL COMMENT 'kind of thing being dealt in',
  `risk` float DEFAULT NULL COMMENT 'suspiciousness rating',
  `risks` bigint(20) UNSIGNED NOT NULL DEFAULT 0 COMMENT 'list of risk factors',
  `recursId` bigint(20) DEFAULT NULL COMMENT 'related record ID in tx_timed, for recurring or delayed transaction',
  `reversesXid` bigint(20) DEFAULT NULL COMMENT 'xid of the transaction this one reverses (if any)',
  `created` bigint(20) NOT NULL DEFAULT 0 COMMENT 'Unixtime transaction was created',
  `deleted` bigint(20) DEFAULT NULL COMMENT 'Unixtime transaction was deleted, null if it has not been'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Record of all rCredits transactions in the region' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Stand-in structure for view `tx_requests`
-- (See below for the actual view)
--
CREATE TABLE `tx_requests` (
`nvid` bigint(20)
,`status` int(11)
,`amount` decimal(11,2)
,`payer` bigint(20)
,`payee` bigint(20)
,`goods` tinyint(4)
,`purpose` longtext
,`cat` bigint(20)
,`flags` bigint(20) unsigned
,`data` longtext
,`reversesXid` bigint(20)
,`recursId` bigint(20)
,`created` int(11)
,`deleted` bigint(20)
);

-- --------------------------------------------------------

--
-- Table structure for table `tx_requests_all`
--

CREATE TABLE `tx_requests_all` (
  `nvid` bigint(20) NOT NULL COMMENT 'the unique invoice ID',
  `status` int(11) NOT NULL DEFAULT -1 COMMENT 'transaction record ID or status (approved, pending, denied, or paid)',
  `amount` decimal(11,2) DEFAULT NULL COMMENT 'amount to charge (negative for store credit)',
  `payer` bigint(20) DEFAULT NULL COMMENT 'user id of the payer',
  `payee` bigint(20) DEFAULT NULL COMMENT 'user id of the payee',
  `goods` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'is this an invoice for real goods and services?',
  `purpose` longtext DEFAULT NULL COMMENT 'payee''s description',
  `cat` bigint(20) DEFAULT NULL COMMENT 'related tx_cats record ID',
  `flags` bigint(20) UNSIGNED NOT NULL DEFAULT 0 COMMENT 'boolean characteristics and state flags',
  `data` longtext DEFAULT NULL COMMENT 'miscellaneous non-searchable data (serialized array)',
  `reversesXid` bigint(20) DEFAULT NULL COMMENT 'xid of the transaction this invoice reverses (if any)',
  `recursId` bigint(20) DEFAULT NULL COMMENT 'related record ID in tx_timed, for recurring or delayed charge (or reversed payment)',
  `created` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime invoice was created',
  `deleted` bigint(20) DEFAULT NULL COMMENT 'record is deleted'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Record of all rCredits invoices in the region' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `tx_rules`
--

CREATE TABLE `tx_rules` (
  `id` int(11) NOT NULL,
  `action` enum('pay','charge','surtx','redeem') NOT NULL COMMENT 'Action that triggers templates of this type',
  `from` bigint(20) NOT NULL COMMENT 'Who to transfer money from',
  `to` bigint(20) NOT NULL COMMENT 'Who to transfer money to',
  `amount` decimal(11,2) UNSIGNED NOT NULL DEFAULT 0.00 COMMENT 'Fixed amount to transfer',
  `portion` decimal(7,6) UNSIGNED NOT NULL DEFAULT 0.000000 COMMENT 'Proportional amount, e.g., 5%, to transfer, expressed as a decimal, e.g., 0.05',
  `purpose` varchar(255) NOT NULL DEFAULT '' COMMENT 'Text to appear on statements explaining this',
  `payerType` enum('anybody','account','anyCo','industry','group','person') NOT NULL COMMENT 'Type of payer',
  `payer` bigint(20) DEFAULT NULL COMMENT 'Payer party to base transaction, null if anybody',
  `payeeType` enum('anybody','account','anyCo','industry','group','person') NOT NULL COMMENT 'Type of payee',
  `payee` bigint(20) DEFAULT NULL COMMENT 'Payee party to base transaction, null if anybody',
  `minimum` decimal(11,2) UNSIGNED NOT NULL DEFAULT 0.00 COMMENT 'Minimum amount of transaction that this template applies to',
  `useMax` int(11) UNSIGNED DEFAULT NULL COMMENT 'Maximum number of uses per member, NULL if no max',
  `amtMax` decimal(11,2) UNSIGNED DEFAULT NULL COMMENT 'Maximum amount to transfer per rule (if portion=0) or per transaction (if portion<>0), NULL if no limit',
  `start` bigint(20) NOT NULL COMMENT 'Start of period for which this occurrence applies',
  `end` bigint(20) DEFAULT NULL COMMENT 'End of period for which this occurrence applies, NULL if it does not end',
  `code` int(11) DEFAULT NULL COMMENT 'For gift cards the individual code',
  `template` int(11) DEFAULT NULL COMMENT 'Template of which this is an occurrence'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Occurrences of a rule' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `tx_timed`
--

CREATE TABLE `tx_timed` (
  `id` int(11) NOT NULL,
  `action` enum('pay','charge','surtx','redeem') NOT NULL COMMENT 'Action that triggers templates of this type',
  `from` bigint(20) NOT NULL COMMENT 'Who to transfer money from',
  `to` bigint(20) NOT NULL COMMENT 'Who to transfer money to',
  `amount` decimal(11,2) UNSIGNED DEFAULT 0.00 COMMENT 'Fixed amount to transfer (NULL for sweep)',
  `portion` decimal(7,6) UNSIGNED NOT NULL DEFAULT 0.000000 COMMENT 'Proportional amount, e.g., 5%, to transfer, expressed as a decimal, e.g., 0.05',
  `purpose` varchar(255) NOT NULL DEFAULT '' COMMENT 'Text to appear on statements explaining this',
  `payerType` enum('anybody','account','anyCo','industry','group','person') NOT NULL COMMENT 'Type of payer',
  `payer` bigint(20) DEFAULT NULL COMMENT 'Payer party to base transaction, null if anybody',
  `payeeType` enum('anybody','account','anyCo','industry','group','person') NOT NULL COMMENT 'Type of payee',
  `payee` bigint(20) DEFAULT NULL COMMENT 'Payee party to base transaction, null if anybody',
  `minimum` decimal(11,2) UNSIGNED NOT NULL DEFAULT 0.00 COMMENT 'Minimum amount of transaction that this template applies to',
  `useMax` int(11) UNSIGNED DEFAULT NULL COMMENT 'Maximum number of uses per member, NULL if no max',
  `amtMax` decimal(11,2) UNSIGNED DEFAULT NULL COMMENT 'Maximum amount to transfer per rule (if portion=0) or per transaction (if portion<>0), NULL if no limit',
  `flags` bigint(20) NOT NULL DEFAULT 0 COMMENT 'transaction flag bits',
  `start` bigint(20) NOT NULL COMMENT 'Start date of first occurrence of this template',
  `end` bigint(20) DEFAULT NULL COMMENT 'Date after which no more occurrences will be created (NULL if no end)',
  `period` enum('once','day','week','month','quarter','year','forever') NOT NULL COMMENT 'The units for the period',
  `periods` int(11) UNSIGNED NOT NULL DEFAULT 1 COMMENT 'Every how many periods a rule will be generated',
  `duration` enum('once','day','week','month','quarter','year','forever') NOT NULL COMMENT 'The unit of duration',
  `durations` int(11) UNSIGNED NOT NULL DEFAULT 1 COMMENT 'How many duration units an occurrence is valid for'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Templates for auxiliary transactions' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `users`
--

CREATE TABLE `users` (
  `uid` bigint(20) NOT NULL COMMENT 'account record ID',
  `name` varchar(60) DEFAULT NULL COMMENT 'unique user name',
  `pass` varchar(128) DEFAULT NULL COMMENT 'account password',
  `email` varchar(254) DEFAULT NULL COMMENT 'email address',
  `created` int(11) NOT NULL DEFAULT 0 COMMENT 'Unix date and time record was created',
  `access` int(11) NOT NULL DEFAULT 0 COMMENT 'Unix date and time account was last accessed',
  `login` int(11) NOT NULL DEFAULT 0 COMMENT 'Unix date and time user last signed in',
  `picture` int(11) NOT NULL DEFAULT 0 COMMENT 'used for temporary storage when generating statistics',
  `data` longtext DEFAULT NULL COMMENT 'serialized associative array of miscellaneous fields (not encrypted)',
  `flags` bigint(20) UNSIGNED NOT NULL DEFAULT 0 COMMENT 'boolean permissions and state flags',
  `jid` bigint(20) NOT NULL DEFAULT 0 COMMENT 'account ID of joined account (0 if none)',
  `steps` bigint(20) UNSIGNED NOT NULL DEFAULT 0 COMMENT 'boolean account setup steps completed',
  `changes` longblob DEFAULT NULL COMMENT 'changes made to the account',
  `community` bigint(20) NOT NULL DEFAULT 0 COMMENT 'account ID of this account''s Common Good Community',
  `secure` mediumblob DEFAULT NULL COMMENT 'encrypted data',
  `vsecure` blob DEFAULT NULL COMMENT 'hyper-encrypted data',
  `fullName` varchar(255) DEFAULT NULL COMMENT 'full name of the individual or entity',
  `phone` varchar(255) DEFAULT NULL COMMENT 'contact phone (no country code, no punctuation)',
  `city` varchar(60) DEFAULT NULL COMMENT 'municipality',
  `state` int(5) NOT NULL DEFAULT 0 COMMENT 'state/province index',
  `zip` varchar(255) DEFAULT NULL COMMENT 'postal code for physical address (no punctuation)',
  `country` int(4) NOT NULL DEFAULT 0 COMMENT 'country index',
  `latitude` decimal(11,8) NOT NULL DEFAULT 0.00000000 COMMENT 'latitude of account''s physical address',
  `longitude` decimal(11,8) NOT NULL DEFAULT 0.00000000 COMMENT 'longitude of account''s physical address',
  `notes` longtext DEFAULT NULL COMMENT 'miscellaneous notes about the user or the account',
  `tickle` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime to tickle an admin about this account',
  `activated` int(11) NOT NULL DEFAULT 0 COMMENT 'when was the account activated',
  `helper` bigint(20) NOT NULL DEFAULT 0 COMMENT 'account that invited this person or company',
  `iCode` int(11) NOT NULL DEFAULT 0 COMMENT 'sequence number of helper invitation',
  `signed` int(11) NOT NULL DEFAULT 0 COMMENT 'when did this person sign the Common Good Agreement',
  `signedBy` varchar(60) DEFAULT NULL COMMENT 'who signed the agreement (on behalf of the account)',
  `savingsAdd` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'chosen amount to hold as savings, beyond rewards',
  `saveWeekly` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'chosen amount to increase minimum (target balance) by, weekly',
  `floor` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'negative credit line',
  `minimum` decimal(11,2) DEFAULT NULL COMMENT 'chosen target balance (for automatic refills)',
  `crumbs` decimal(6,3) NOT NULL DEFAULT 0.000 COMMENT 'percentage of each transaction to donate to CG',
  `backing` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'amount account-holder chose to back',
  `backingDate` bigint(20) NOT NULL DEFAULT 0 COMMENT 'date account-holder started backing',
  `backingNext` decimal(11,2) DEFAULT NULL COMMENT 'lower backing amount for the next year',
  `food` decimal(6,3) NOT NULL DEFAULT 0.000 COMMENT 'percentage of each food purchase to donate to the food fund',
  `balance` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'balance, not including rewards (cached)',
  `rewards` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'total incentive rewards to date (cached)',
  `committed` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'amount committed (for donations to CGF)',
  `risk` float DEFAULT NULL COMMENT 'today''s suspiciousness rating',
  `risks` bigint(20) UNSIGNED NOT NULL DEFAULT 0 COMMENT 'list of risk factors',
  `trust` float NOT NULL DEFAULT 0 COMMENT 'how much this person is trusted by others in the community (0 for companies)',
  `stats` longtext DEFAULT NULL COMMENT 'account statistics',
  `notices` text DEFAULT NULL COMMENT 'when to send what kind of notice',
  `lastip` varchar(39) DEFAULT NULL COMMENT 'latest IP address used',
  `preid` bigint(20) NOT NULL DEFAULT 0 COMMENT 'signup record ID',
  `special` longtext DEFAULT NULL COMMENT 'special transient data',
  `source` mediumtext DEFAULT NULL COMMENT 'how did the member hear about us?'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Stores user data.' ROW_FORMAT=DYNAMIC;

--
-- Dumping data for table `users`
--

INSERT INTO `users` VALUES(0, NULL, NULL, NULL, 0, 0, 0, 0, NULL, 0, 0, 0, NULL, 0, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0, 0.00000000, 0.00000000, NULL, 0, 0, 0, 0, 0, NULL, 0.00, 0.00, 0.00, NULL, 0.000, 0.00, 0, NULL, 0.000, 0.00, 0.00, 0.00, NULL, 0, 0, NULL, NULL, NULL, 0, NULL, NULL);

-- --------------------------------------------------------

--
-- Table structure for table `u_company`
--

CREATE TABLE `u_company` (
  `uid` bigint(20) NOT NULL COMMENT 'account record ID',
  `coType` tinyint(4) DEFAULT NULL COMMENT 'type of entity',
  `contact` varchar(255) DEFAULT NULL COMMENT 'whom to contact about this account',
  `website` text DEFAULT NULL COMMENT 'company website domain',
  `description` longtext DEFAULT NULL COMMENT 'long markdown description',
  `shortDesc` text DEFAULT NULL COMMENT 'one line description',
  `selling` longtext DEFAULT NULL COMMENT 'list of typical transaction descriptions',
  `coFlags` bigint(20) UNSIGNED NOT NULL DEFAULT 0 COMMENT 'miscellaneous flag bits',
  `founded` bigint(20) DEFAULT NULL COMMENT 'date the company was founded',
  `gross` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'average annual gross receipts',
  `zips` text DEFAULT NULL COMMENT 'zip regex for geographic region the company serves',
  `employees` int(11) NOT NULL DEFAULT 0 COMMENT 'number of employees',
  `staleNudge` int(11) NOT NULL DEFAULT 7 COMMENT 'how many days to wait before reminding customer to pay',
  `payrollStart` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime date last payroll started',
  `payrollEnd` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime date last payroll ended',
  `mission` longtext DEFAULT NULL COMMENT 'the organization''s mission',
  `activities` longtext DEFAULT NULL COMMENT 'what the (sponsored) organization actually does to advance its mission',
  `checksIn` mediumint(8) DEFAULT NULL COMMENT 'expected number of checks received monthly',
  `checksOut` mediumint(8) DEFAULT NULL COMMENT 'expected number of outgoing payments monthly',
  `logo` text DEFAULT NULL COMMENT 'company logo URL',
  `target` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'fundraising target amount',
  `targetStart` int(11) DEFAULT NULL COMMENT 'starting date of fundraising project'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Companies' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `u_groupies`
--

CREATE TABLE `u_groupies` (
  `id` int(11) NOT NULL,
  `uid` bigint(20) NOT NULL,
  `grpId` int(11) NOT NULL,
  `isMember` tinyint(1) NOT NULL DEFAULT 0 COMMENT 'User is a member of the group',
  `canAdd` tinyint(1) NOT NULL DEFAULT 0 COMMENT 'User can add other users to the group',
  `canRemove` tinyint(1) NOT NULL DEFAULT 0 COMMENT 'User can remove other users from the group',
  `start` bigint(20) NOT NULL,
  `end` bigint(20) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `u_groups`
--

CREATE TABLE `u_groups` (
  `id` int(11) NOT NULL,
  `name` varchar(40) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `u_photo`
--

CREATE TABLE `u_photo` (
  `uid` bigint(20) NOT NULL COMMENT 'account record id',
  `photo` longblob DEFAULT NULL COMMENT 'member photo',
  `thumb` blob DEFAULT NULL COMMENT 'small version of photo'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='one photo for each account' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `u_relations`
--

CREATE TABLE `u_relations` (
  `reid` bigint(20) NOT NULL COMMENT 'relationship record id',
  `main` bigint(20) DEFAULT NULL COMMENT 'uid of the account to which others are related',
  `other` bigint(20) DEFAULT NULL COMMENT 'uid of an other account related to this account',
  `otherNum` int(11) NOT NULL DEFAULT 0 COMMENT 'sequence number of the other (starts with 1)',
  `permission` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'what type of permission the other has on the main account',
  `code` varchar(50) DEFAULT NULL COMMENT 'the (main) company''s account ID for this other',
  `data` longtext DEFAULT NULL COMMENT 'serialized array of parameters',
  `flags` bigint(20) UNSIGNED NOT NULL DEFAULT 0 COMMENT 'boolean type flags',
  `created` int(11) DEFAULT NULL COMMENT 'Unixtime record created'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Who can manage which accounts, and how' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `u_shout`
--

CREATE TABLE `u_shout` (
  `uid` bigint(20) NOT NULL COMMENT 'record ID in users table',
  `org` varchar(255) DEFAULT NULL COMMENT 'signer''s organization, if any',
  `title` varchar(255) DEFAULT NULL COMMENT 'signer''s title',
  `website` varchar(255) DEFAULT NULL COMMENT 'organization website, if any',
  `quote` longtext DEFAULT NULL COMMENT 'what benefit this signer sees for the community',
  `created` int(11) DEFAULT NULL COMMENT 'creation date',
  `usePhoto` enum('0','1') NOT NULL COMMENT 'okay to use this member''s photo in a publicity collage?',
  `postPhoto` enum('0','1') NOT NULL COMMENT 'okay to use this member''s photo in social media?',
  `sawVideo` enum('0','1') NOT NULL COMMENT 'did the member see the video?',
  `rating` int(11) NOT NULL DEFAULT 0 COMMENT 'how awesome is the quote, 0=not'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='people who have signed a public statement of support' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `u_track`
--

CREATE TABLE `u_track` (
  `id` bigint(20) NOT NULL COMMENT 'record ID',
  `uid` bigint(20) DEFAULT NULL COMMENT 'related account record ID',
  `type` varchar(255) DEFAULT NULL COMMENT 'what type of email or email address (for invite)',
  `sent` varchar(11) DEFAULT NULL COMMENT 'latest date sent',
  `seen` varchar(11) DEFAULT NULL COMMENT 'latest date opened'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='contact information for non-members' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `variable`
--

CREATE TABLE `variable` (
  `name` varchar(128) NOT NULL DEFAULT '' COMMENT 'The name of the variable.',
  `value` longblob NOT NULL COMMENT 'The value of the variable.'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Named variable/value pairs created by Drupal core or any...' ROW_FORMAT=DYNAMIC;

--
-- Dumping data for table `variable`
--

INSERT INTO `variable` VALUES('menu_expanded', 0x613a303a7b7d);
INSERT INTO `variable` VALUES('menu_masks', 0x613a31303a7b693a303b693a3132353b693a313b693a3132313b693a323b693a36333b693a333b693a36323b693a343b693a36303b693a353b693a33313b693a363b693a31353b693a373b693a373b693a383b693a333b693a393b693a313b7d);
INSERT INTO `variable` VALUES('up', 0x623a313b);

-- --------------------------------------------------------

--
-- Table structure for table `x_company`
--

CREATE TABLE `x_company` (
  `deleted` bigint(20) DEFAULT NULL COMMENT 'Unixtime record was deleted',
  `uid` bigint(20) NOT NULL COMMENT 'account record ID',
  `coType` tinyint(4) DEFAULT NULL COMMENT 'type of entity',
  `contact` varchar(255) DEFAULT NULL COMMENT 'whom to contact about this account',
  `website` text DEFAULT NULL COMMENT 'company website domain',
  `description` longtext DEFAULT NULL COMMENT 'long markdown description',
  `shortDesc` text DEFAULT NULL COMMENT 'one line description',
  `selling` longtext DEFAULT NULL COMMENT 'list of typical transaction descriptions',
  `coFlags` bigint(20) UNSIGNED NOT NULL DEFAULT 0 COMMENT 'miscellaneous flag bits',
  `founded` int(11) DEFAULT NULL COMMENT 'date the company was founded',
  `gross` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'average annual gross receipts',
  `zips` text DEFAULT NULL COMMENT 'zip regex for geographic region the company serves',
  `employees` int(11) NOT NULL DEFAULT 0 COMMENT 'number of employees',
  `staleNudge` int(11) NOT NULL DEFAULT 7 COMMENT 'how many days to wait before reminding customer to pay',
  `payrollStart` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime date last payroll started',
  `payrollEnd` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime date last payroll ended',
  `mission` longtext DEFAULT NULL COMMENT 'the organization''s mission',
  `activities` longtext DEFAULT NULL COMMENT 'what the (sponsored) organization actually does to advance its mission',
  `checksIn` mediumint(8) DEFAULT NULL COMMENT 'expected number of checks received monthly',
  `checksOut` mediumint(8) DEFAULT NULL COMMENT 'expected number of outgoing payments monthly',
  `logo` text DEFAULT NULL COMMENT 'company logo URL',
  `target` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'fundraising target amount',
  `targetStart` int(11) DEFAULT NULL COMMENT 'starting date of fundraising project'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Companies' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `x_photo`
--

CREATE TABLE `x_photo` (
  `deleted` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime record was deleted',
  `uid` bigint(20) NOT NULL COMMENT 'account record id',
  `photo` longblob DEFAULT NULL COMMENT 'member photo',
  `thumb` blob DEFAULT NULL COMMENT 'small version of photo'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='one photo for each account' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `x_relations`
--

CREATE TABLE `x_relations` (
  `deleted` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime record was deleted',
  `reid` bigint(20) NOT NULL COMMENT 'relationship record id',
  `main` bigint(20) DEFAULT NULL COMMENT 'uid of the account to which others are related',
  `other` bigint(20) DEFAULT NULL COMMENT 'uid of an other account related to this account',
  `otherNum` int(11) NOT NULL DEFAULT 0 COMMENT 'sequence number of the other (starts with 1)',
  `permission` tinyint(4) NOT NULL DEFAULT 0 COMMENT 'what type of permission the other has on the main account',
  `code` varchar(50) DEFAULT NULL COMMENT 'the (main) company''s account ID for this other',
  `data` longtext DEFAULT NULL COMMENT 'serialized array of parameters',
  `flags` bigint(20) UNSIGNED NOT NULL DEFAULT 0 COMMENT 'boolean type flags',
  `created` int(11) DEFAULT NULL COMMENT 'Unixtime record created'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Who can manage which accounts, and how' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `x_shout`
--

CREATE TABLE `x_shout` (
  `deleted` bigint(20) DEFAULT NULL COMMENT 'Unixtime record was deleted',
  `uid` bigint(20) NOT NULL COMMENT 'record ID in users table',
  `org` varchar(255) DEFAULT NULL COMMENT 'signer''s organization, if any',
  `title` varchar(255) DEFAULT NULL COMMENT 'signer''s title',
  `website` varchar(255) DEFAULT NULL COMMENT 'organization website, if any',
  `quote` longtext DEFAULT NULL COMMENT 'what benefit this signer sees for the community',
  `created` int(11) DEFAULT NULL COMMENT 'creation date',
  `usePhoto` enum('0','1') NOT NULL COMMENT 'okay to use this member''s photo in a publicity collage?',
  `postPhoto` enum('0','1') NOT NULL COMMENT 'okay to use this member''s photo in social media?',
  `sawVideo` enum('0','1') NOT NULL COMMENT 'did the member see the video?',
  `rating` int(11) NOT NULL DEFAULT 0 COMMENT 'how awesome is the quote, 0=not'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='people who have signed a public statement of support' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `x_txs2`
--

CREATE TABLE `x_txs2` (
  `deleted` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime record was deleted',
  `txid` bigint(20) NOT NULL COMMENT 'the unique transaction ID',
  `amount` decimal(11,2) DEFAULT NULL COMMENT 'amount of transfer',
  `payee` bigint(20) DEFAULT NULL COMMENT 'CG account record ID',
  `created` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime transaction was created',
  `completed` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime transaction was completed',
  `deposit` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime transfer check was printed and deposited',
  `bankAccount` blob DEFAULT NULL COMMENT 'Bank account for the transfer',
  `isSavings` tinyint(4) DEFAULT NULL COMMENT '1 if bankAccount is a savings account',
  `risk` float DEFAULT NULL COMMENT 'suspiciousness rating',
  `risks` bigint(20) UNSIGNED NOT NULL DEFAULT 0 COMMENT 'list of risk factors',
  `bankTxId` bigint(20) DEFAULT NULL COMMENT 'bank transaction ID',
  `channel` tinyint(4) DEFAULT NULL COMMENT 'through what medium was the transaction entered',
  `xid` bigint(20) NOT NULL DEFAULT 0 COMMENT 'id of related tx_hdrs record',
  `pid` bigint(20) DEFAULT NULL COMMENT 'related people record ID'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Record of USD (Dwolla) transactions in the region' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `x_users`
--

CREATE TABLE `x_users` (
  `deleted` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime record was deleted',
  `uid` bigint(20) NOT NULL COMMENT 'account record ID',
  `name` varchar(60) DEFAULT NULL COMMENT 'unique user name',
  `pass` varchar(128) DEFAULT NULL COMMENT 'account password',
  `email` varchar(254) DEFAULT NULL COMMENT 'email address',
  `created` int(11) NOT NULL DEFAULT 0 COMMENT 'Unix date and time record was created',
  `access` int(11) NOT NULL DEFAULT 0 COMMENT 'Unix date and time account was last accessed',
  `login` int(11) NOT NULL DEFAULT 0 COMMENT 'Unix date and time user last signed in',
  `picture` int(11) NOT NULL DEFAULT 0 COMMENT 'used for temporary storage when generating statistics',
  `data` longtext DEFAULT NULL COMMENT 'serialized associative array of miscellaneous fields (not encrypted)',
  `flags` bigint(20) UNSIGNED NOT NULL DEFAULT 0 COMMENT 'boolean permissions and state flags',
  `jid` bigint(20) NOT NULL DEFAULT 0 COMMENT 'account ID of joined account (0 if none)',
  `steps` bigint(20) UNSIGNED NOT NULL DEFAULT 0 COMMENT 'boolean account setup steps completed',
  `changes` longblob DEFAULT NULL COMMENT 'changes made to the account',
  `community` bigint(20) NOT NULL DEFAULT 0 COMMENT 'account ID of this account''s Common Good Community',
  `secure` mediumblob DEFAULT NULL COMMENT 'encrypted data',
  `vsecure` blob DEFAULT NULL COMMENT 'hyper-encrypted data',
  `fullName` varchar(255) DEFAULT NULL COMMENT 'full name of the individual or entity',
  `phone` varchar(255) DEFAULT NULL COMMENT 'contact phone (no country code, no punctuation)',
  `city` varchar(60) DEFAULT NULL COMMENT 'municipality',
  `state` int(5) NOT NULL DEFAULT 0 COMMENT 'state/province index',
  `zip` varchar(255) DEFAULT NULL COMMENT 'postal code for physical address (no punctuation)',
  `country` int(4) NOT NULL DEFAULT 0 COMMENT 'country index',
  `latitude` decimal(11,8) NOT NULL DEFAULT 0.00000000 COMMENT 'latitude of account''s physical address',
  `longitude` decimal(11,8) NOT NULL DEFAULT 0.00000000 COMMENT 'longitude of account''s physical address',
  `notes` longtext DEFAULT NULL COMMENT 'miscellaneous notes about the user or the account',
  `tickle` int(11) NOT NULL DEFAULT 0 COMMENT 'Unixtime to tickle an admin about this account',
  `activated` int(11) NOT NULL DEFAULT 0 COMMENT 'when was the account activated',
  `helper` bigint(20) NOT NULL DEFAULT 0 COMMENT 'account that invited this person or company',
  `iCode` int(11) NOT NULL DEFAULT 0 COMMENT 'sequence number of helper invitation',
  `signed` int(11) NOT NULL DEFAULT 0 COMMENT 'when did this person sign the Common Good Agreement',
  `signedBy` varchar(60) DEFAULT NULL COMMENT 'who signed the agreement (on behalf of the account)',
  `savingsAdd` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'chosen amount to hold as savings, beyond rewards',
  `saveWeekly` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'chosen amount to increase minimum (target balance) by, weekly',
  `floor` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'negative credit line',
  `minimum` decimal(11,2) DEFAULT NULL COMMENT 'chosen target balance (for automatic refills)',
  `crumbs` decimal(6,3) NOT NULL DEFAULT 0.000 COMMENT 'percentage of each transaction to donate to CG',
  `backing` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'amount account-holder chose to back',
  `backingDate` bigint(20) NOT NULL DEFAULT 0 COMMENT 'date account-holder started backing',
  `backingNext` decimal(11,2) DEFAULT NULL COMMENT 'lower backing amount for the next year',
  `food` decimal(6,3) NOT NULL DEFAULT 0.000 COMMENT 'percentage of each food purchase to donate to the food fund',
  `balance` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'balance, not including rewards (cached)',
  `rewards` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'total incentive rewards to date (cached)',
  `committed` decimal(11,2) NOT NULL DEFAULT 0.00 COMMENT 'amount committed (for donations to CGF)',
  `risk` float DEFAULT NULL COMMENT 'today''s suspiciousness rating',
  `risks` bigint(20) UNSIGNED NOT NULL DEFAULT 0 COMMENT 'list of risk factors',
  `trust` float NOT NULL DEFAULT 0 COMMENT 'how much this person is trusted by others in the community (0 for companies)',
  `stats` longtext DEFAULT NULL COMMENT 'account statistics',
  `notices` text DEFAULT NULL COMMENT 'when to send what kind of notice',
  `lastip` varchar(39) DEFAULT NULL COMMENT 'latest IP address used',
  `preid` bigint(20) NOT NULL DEFAULT 0 COMMENT 'signup record ID',
  `special` longtext DEFAULT NULL COMMENT 'special transient data',
  `source` mediumtext DEFAULT NULL COMMENT 'how did the member hear about us?'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Stores user data.' ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `zip3`
--

CREATE TABLE `zip3` (
  `id` varchar(3) NOT NULL COMMENT 'first 3 digits of Zip Code',
  `region` varchar(255) DEFAULT NULL COMMENT 'city or region within a state',
  `state` varchar(2) DEFAULT NULL COMMENT 'state abbreviation'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='meaning of first 3 digits of Zip Codes' ROW_FORMAT=DYNAMIC;

--
-- Dumping data for table `zip3`
--

INSERT INTO `zip3` VALUES('005', 'Mid-Island', 'NY');
INSERT INTO `zip3` VALUES('006', 'San Juan', 'PR');
INSERT INTO `zip3` VALUES('007', 'San Juan', 'PR');
INSERT INTO `zip3` VALUES('008', 'San Juan', 'PR');
INSERT INTO `zip3` VALUES('009', 'San Juan', 'PR');
INSERT INTO `zip3` VALUES('010', 'Springfield', 'MA');
INSERT INTO `zip3` VALUES('011', 'Springfield', 'MA');
INSERT INTO `zip3` VALUES('012', 'Pittsfield', 'MA');
INSERT INTO `zip3` VALUES('013', 'Springfield', 'MA');
INSERT INTO `zip3` VALUES('014', 'Central', 'MA');
INSERT INTO `zip3` VALUES('015', 'Central', 'MA');
INSERT INTO `zip3` VALUES('016', 'Worcester', 'MA');
INSERT INTO `zip3` VALUES('017', 'Central', 'MA');
INSERT INTO `zip3` VALUES('018', 'Middlesex-Esx', 'MA');
INSERT INTO `zip3` VALUES('019', 'Middlesex-Esx', 'MA');
INSERT INTO `zip3` VALUES('020', 'Brockton', 'MA');
INSERT INTO `zip3` VALUES('021', 'Boston', 'MA');
INSERT INTO `zip3` VALUES('022', 'Boston', 'MA');
INSERT INTO `zip3` VALUES('023', 'Brockton', 'MA');
INSERT INTO `zip3` VALUES('024', 'Northwest Bos', 'MA');
INSERT INTO `zip3` VALUES('025', 'Cape Cod', 'MA');
INSERT INTO `zip3` VALUES('026', 'Cape Cod', 'MA');
INSERT INTO `zip3` VALUES('027', 'Providence', 'RI');
INSERT INTO `zip3` VALUES('028', 'Providence', 'RI');
INSERT INTO `zip3` VALUES('029', 'Providence', 'RI');
INSERT INTO `zip3` VALUES('030', 'Manchester', 'NH');
INSERT INTO `zip3` VALUES('031', 'Manchester', 'NH');
INSERT INTO `zip3` VALUES('032', 'Manchester', 'NH');
INSERT INTO `zip3` VALUES('033', 'Concord', 'NH');
INSERT INTO `zip3` VALUES('034', 'Manchester', 'NH');
INSERT INTO `zip3` VALUES('035', 'White Riv Jct', 'VT');
INSERT INTO `zip3` VALUES('036', 'White Riv Jct', 'VT');
INSERT INTO `zip3` VALUES('037', 'White Riv Jct', 'VT');
INSERT INTO `zip3` VALUES('038', 'Portsmouth', 'NH');
INSERT INTO `zip3` VALUES('039', 'Portsmouth', 'NH');
INSERT INTO `zip3` VALUES('040', 'Portland', 'ME');
INSERT INTO `zip3` VALUES('041', 'Portland', 'ME');
INSERT INTO `zip3` VALUES('042', 'Portland', 'ME');
INSERT INTO `zip3` VALUES('043', 'Portland', 'ME');
INSERT INTO `zip3` VALUES('044', 'Bangor', 'ME');
INSERT INTO `zip3` VALUES('045', 'Portland', 'ME');
INSERT INTO `zip3` VALUES('046', 'Bangor', 'ME');
INSERT INTO `zip3` VALUES('047', 'Bangor', 'ME');
INSERT INTO `zip3` VALUES('048', 'Portland', 'ME');
INSERT INTO `zip3` VALUES('049', 'Bangor', 'ME');
INSERT INTO `zip3` VALUES('050', 'White Riv Jct', 'VT');
INSERT INTO `zip3` VALUES('051', 'White Riv Jct', 'VT');
INSERT INTO `zip3` VALUES('052', 'White Riv Jct', 'VT');
INSERT INTO `zip3` VALUES('053', 'White Riv Jct', 'VT');
INSERT INTO `zip3` VALUES('054', 'Burlington', 'VT');
INSERT INTO `zip3` VALUES('055', 'Middlesex-Esx', 'MA');
INSERT INTO `zip3` VALUES('056', 'Burlington', 'VT');
INSERT INTO `zip3` VALUES('057', 'White Riv Jct', 'VT');
INSERT INTO `zip3` VALUES('058', 'White Riv Jct', 'VT');
INSERT INTO `zip3` VALUES('059', 'White Riv Jct', 'VT');
INSERT INTO `zip3` VALUES('060', 'Hartford', 'CT');
INSERT INTO `zip3` VALUES('061', 'Hartford', 'CT');
INSERT INTO `zip3` VALUES('062', 'Hartford', 'CT');
INSERT INTO `zip3` VALUES('063', 'Southern', 'CT');
INSERT INTO `zip3` VALUES('064', 'Southern', 'CT');
INSERT INTO `zip3` VALUES('065', 'New Haven', 'CT');
INSERT INTO `zip3` VALUES('066', 'Bridgeport', 'CT');
INSERT INTO `zip3` VALUES('067', 'Waterbury', 'CT');
INSERT INTO `zip3` VALUES('068', 'Stamford', 'CT');
INSERT INTO `zip3` VALUES('069', 'Stamford', 'CT');
INSERT INTO `zip3` VALUES('070', 'Newark', 'NJ');
INSERT INTO `zip3` VALUES('071', 'Newark', 'NJ');
INSERT INTO `zip3` VALUES('072', 'Elizabeth', 'NJ');
INSERT INTO `zip3` VALUES('073', 'Jersey City', 'NJ');
INSERT INTO `zip3` VALUES('074', 'Paterson', 'NJ');
INSERT INTO `zip3` VALUES('075', 'Paterson', 'NJ');
INSERT INTO `zip3` VALUES('076', 'Hackensack', 'NJ');
INSERT INTO `zip3` VALUES('077', 'Monmouth', 'NJ');
INSERT INTO `zip3` VALUES('078', 'West Jersey', 'NJ');
INSERT INTO `zip3` VALUES('079', 'West Jersey', 'NJ');
INSERT INTO `zip3` VALUES('080', 'South Jersey', 'NJ');
INSERT INTO `zip3` VALUES('081', 'Camden', 'NJ');
INSERT INTO `zip3` VALUES('082', 'South Jersey', 'NJ');
INSERT INTO `zip3` VALUES('083', 'South Jersey', 'NJ');
INSERT INTO `zip3` VALUES('084', 'Atlantic City', 'NJ');
INSERT INTO `zip3` VALUES('085', 'Trenton', 'NJ');
INSERT INTO `zip3` VALUES('086', 'Trenton', 'NJ');
INSERT INTO `zip3` VALUES('087', 'Monmouth', 'NJ');
INSERT INTO `zip3` VALUES('088', 'Kilmer', 'NJ');
INSERT INTO `zip3` VALUES('089', 'New Brunswick', 'NJ');
INSERT INTO `zip3` VALUES('090', 'Apo', 'AE');
INSERT INTO `zip3` VALUES('091', 'Apo', 'AE');
INSERT INTO `zip3` VALUES('092', 'Apo', 'AE');
INSERT INTO `zip3` VALUES('093', 'Apo', 'AE');
INSERT INTO `zip3` VALUES('094', 'Apo/Fpo', 'AE');
INSERT INTO `zip3` VALUES('095', 'Fpo', 'AE');
INSERT INTO `zip3` VALUES('096', 'Apo/Fpo', 'AE');
INSERT INTO `zip3` VALUES('097', 'Apo/Fpo', 'AE');
INSERT INTO `zip3` VALUES('098', 'Apo/Fpo', 'AE');
INSERT INTO `zip3` VALUES('099', 'Apo/Fpo', 'AE');
INSERT INTO `zip3` VALUES('100', 'New York', 'NY');
INSERT INTO `zip3` VALUES('101', 'New York', 'NY');
INSERT INTO `zip3` VALUES('102', 'New York', 'NY');
INSERT INTO `zip3` VALUES('103', 'Staten Island', 'NY');
INSERT INTO `zip3` VALUES('104', 'Bronx', 'NY');
INSERT INTO `zip3` VALUES('105', 'Westchester', 'NY');
INSERT INTO `zip3` VALUES('106', 'White Plains', 'NY');
INSERT INTO `zip3` VALUES('107', 'Yonkers', 'NY');
INSERT INTO `zip3` VALUES('108', 'New Rochelle', 'NY');
INSERT INTO `zip3` VALUES('109', 'Westchester', 'NY');
INSERT INTO `zip3` VALUES('110', 'Queens', 'NY');
INSERT INTO `zip3` VALUES('111', 'Long Island City', 'NY');
INSERT INTO `zip3` VALUES('112', 'Brooklyn', 'NY');
INSERT INTO `zip3` VALUES('113', 'Flushing', 'NY');
INSERT INTO `zip3` VALUES('114', 'Jamaica', 'NY');
INSERT INTO `zip3` VALUES('115', 'Western Nassau', 'NY');
INSERT INTO `zip3` VALUES('116', 'Far Rockaway', 'NY');
INSERT INTO `zip3` VALUES('117', 'Mid-Island', 'NY');
INSERT INTO `zip3` VALUES('118', 'Hicksville', 'NY');
INSERT INTO `zip3` VALUES('119', 'Mid-Island', 'NY');
INSERT INTO `zip3` VALUES('120', 'Albany', 'NY');
INSERT INTO `zip3` VALUES('121', 'Albany', 'NY');
INSERT INTO `zip3` VALUES('122', 'Albany', 'NY');
INSERT INTO `zip3` VALUES('123', 'Schenectady', 'NY');
INSERT INTO `zip3` VALUES('124', 'Mid-Hudson', 'NY');
INSERT INTO `zip3` VALUES('125', 'Mid-Hudson', 'NY');
INSERT INTO `zip3` VALUES('126', 'Poughkeepsie', 'NY');
INSERT INTO `zip3` VALUES('127', 'Mid-Hudson', 'NY');
INSERT INTO `zip3` VALUES('128', 'Glens Falls', 'NY');
INSERT INTO `zip3` VALUES('129', 'Plattsburgh', 'NY');
INSERT INTO `zip3` VALUES('130', 'Syracuse', 'NY');
INSERT INTO `zip3` VALUES('131', 'Syracuse', 'NY');
INSERT INTO `zip3` VALUES('132', 'Syracuse', 'NY');
INSERT INTO `zip3` VALUES('133', 'Utica', 'NY');
INSERT INTO `zip3` VALUES('134', 'Utica', 'NY');
INSERT INTO `zip3` VALUES('135', 'Utica', 'NY');
INSERT INTO `zip3` VALUES('136', 'Watertown', 'NY');
INSERT INTO `zip3` VALUES('137', 'Binghamton', 'NY');
INSERT INTO `zip3` VALUES('138', 'Binghamton', 'NY');
INSERT INTO `zip3` VALUES('139', 'Binghamton', 'NY');
INSERT INTO `zip3` VALUES('140', 'Buffalo', 'NY');
INSERT INTO `zip3` VALUES('141', 'Buffalo', 'NY');
INSERT INTO `zip3` VALUES('142', 'Buffalo', 'NY');
INSERT INTO `zip3` VALUES('143', 'Niagara Falls', 'NY');
INSERT INTO `zip3` VALUES('144', 'Rochester', 'NY');
INSERT INTO `zip3` VALUES('145', 'Rochester', 'NY');
INSERT INTO `zip3` VALUES('146', 'Rochester', 'NY');
INSERT INTO `zip3` VALUES('147', 'Jamestown', 'NY');
INSERT INTO `zip3` VALUES('148', 'Elmira', 'NY');
INSERT INTO `zip3` VALUES('149', 'Elmira', 'NY');
INSERT INTO `zip3` VALUES('150', 'Pittsburgh', 'PA');
INSERT INTO `zip3` VALUES('151', 'Pittsburgh', 'PA');
INSERT INTO `zip3` VALUES('152', 'Pittsburgh', 'PA');
INSERT INTO `zip3` VALUES('153', 'Pittsburgh', 'PA');
INSERT INTO `zip3` VALUES('154', 'Pittsburgh', 'PA');
INSERT INTO `zip3` VALUES('155', 'Johnstown', 'PA');
INSERT INTO `zip3` VALUES('156', 'Greensburg', 'PA');
INSERT INTO `zip3` VALUES('157', 'Johnstown', 'PA');
INSERT INTO `zip3` VALUES('158', 'Du Bois', 'PA');
INSERT INTO `zip3` VALUES('159', 'Johnstown', 'PA');
INSERT INTO `zip3` VALUES('160', 'New Castle', 'PA');
INSERT INTO `zip3` VALUES('161', 'New Castle', 'PA');
INSERT INTO `zip3` VALUES('162', 'New Castle', 'PA');
INSERT INTO `zip3` VALUES('163', 'Oil City', 'PA');
INSERT INTO `zip3` VALUES('164', 'Erie', 'PA');
INSERT INTO `zip3` VALUES('165', 'Erie', 'PA');
INSERT INTO `zip3` VALUES('166', 'Altoona', 'PA');
INSERT INTO `zip3` VALUES('167', 'Bradford', 'PA');
INSERT INTO `zip3` VALUES('168', 'Altoona', 'PA');
INSERT INTO `zip3` VALUES('169', 'Williamsport', 'PA');
INSERT INTO `zip3` VALUES('170', 'Harrisburg', 'PA');
INSERT INTO `zip3` VALUES('171', 'Harrisburg', 'PA');
INSERT INTO `zip3` VALUES('172', 'Harrisburg', 'PA');
INSERT INTO `zip3` VALUES('173', 'Lancaster', 'PA');
INSERT INTO `zip3` VALUES('174', 'York', 'PA');
INSERT INTO `zip3` VALUES('175', 'Lancaster', 'PA');
INSERT INTO `zip3` VALUES('176', 'Lancaster', 'PA');
INSERT INTO `zip3` VALUES('177', 'Williamsport', 'PA');
INSERT INTO `zip3` VALUES('178', 'Harrisburg', 'PA');
INSERT INTO `zip3` VALUES('179', 'Reading', 'PA');
INSERT INTO `zip3` VALUES('180', 'Lehigh Valley', 'PA');
INSERT INTO `zip3` VALUES('181', 'Allentown', 'PA');
INSERT INTO `zip3` VALUES('182', 'Wilkes Barre', 'PA');
INSERT INTO `zip3` VALUES('183', 'Lehigh Valley', 'PA');
INSERT INTO `zip3` VALUES('184', 'Scranton', 'PA');
INSERT INTO `zip3` VALUES('185', 'Scranton', 'PA');
INSERT INTO `zip3` VALUES('186', 'Wilkes Barre', 'PA');
INSERT INTO `zip3` VALUES('187', 'Wilkes Barre', 'PA');
INSERT INTO `zip3` VALUES('188', 'Scranton', 'PA');
INSERT INTO `zip3` VALUES('189', 'Southeastern', 'PA');
INSERT INTO `zip3` VALUES('190', 'Philadelphia', 'PA');
INSERT INTO `zip3` VALUES('191', 'Philadelphia', 'PA');
INSERT INTO `zip3` VALUES('192', 'Philadelphia', 'PA');
INSERT INTO `zip3` VALUES('193', 'Southeastern', 'PA');
INSERT INTO `zip3` VALUES('194', 'Southeastern', 'PA');
INSERT INTO `zip3` VALUES('195', 'Reading', 'PA');
INSERT INTO `zip3` VALUES('196', 'Reading', 'PA');
INSERT INTO `zip3` VALUES('197', 'Wilmington', 'DE');
INSERT INTO `zip3` VALUES('198', 'Wilmington', 'DE');
INSERT INTO `zip3` VALUES('199', 'Wilmington', 'DE');
INSERT INTO `zip3` VALUES('200', 'Washington', 'DC');
INSERT INTO `zip3` VALUES('201', 'Dulles', 'VA');
INSERT INTO `zip3` VALUES('202', 'Washington', 'DC');
INSERT INTO `zip3` VALUES('203', 'Washington', 'DC');
INSERT INTO `zip3` VALUES('204', 'Washington', 'DC');
INSERT INTO `zip3` VALUES('205', 'Washington', 'DC');
INSERT INTO `zip3` VALUES('206', 'Southern Md', 'MD');
INSERT INTO `zip3` VALUES('207', 'Southern Md', 'MD');
INSERT INTO `zip3` VALUES('208', 'Suburban Md', 'MD');
INSERT INTO `zip3` VALUES('209', 'Silver Spring', 'MD');
INSERT INTO `zip3` VALUES('210', 'Linthicum', 'MD');
INSERT INTO `zip3` VALUES('211', 'Linthicum', 'MD');
INSERT INTO `zip3` VALUES('212', 'Baltimore', 'MD');
INSERT INTO `zip3` VALUES('214', 'Annapolis', 'MD');
INSERT INTO `zip3` VALUES('215', 'Cumberland', 'MD');
INSERT INTO `zip3` VALUES('216', 'Eastern Shore', 'MD');
INSERT INTO `zip3` VALUES('217', 'Frederick', 'MD');
INSERT INTO `zip3` VALUES('218', 'Salisbury', 'MD');
INSERT INTO `zip3` VALUES('219', 'Baltimore', 'MD');
INSERT INTO `zip3` VALUES('220', 'Northern Va', 'VA');
INSERT INTO `zip3` VALUES('221', 'Northern Va', 'VA');
INSERT INTO `zip3` VALUES('222', 'Arlington', 'VA');
INSERT INTO `zip3` VALUES('223', 'Alexandria', 'VA');
INSERT INTO `zip3` VALUES('224', 'Richmond', 'VA');
INSERT INTO `zip3` VALUES('225', 'Richmond', 'VA');
INSERT INTO `zip3` VALUES('226', 'Winchester', 'VA');
INSERT INTO `zip3` VALUES('227', 'Culpeper', 'VA');
INSERT INTO `zip3` VALUES('228', 'Charlottesvle', 'VA');
INSERT INTO `zip3` VALUES('229', 'Charlottesvle', 'VA');
INSERT INTO `zip3` VALUES('230', 'Richmond', 'VA');
INSERT INTO `zip3` VALUES('231', 'Richmond', 'VA');
INSERT INTO `zip3` VALUES('232', 'Richmond', 'VA');
INSERT INTO `zip3` VALUES('233', 'Norfolk', 'VA');
INSERT INTO `zip3` VALUES('234', 'Norfolk', 'VA');
INSERT INTO `zip3` VALUES('235', 'Norfolk', 'VA');
INSERT INTO `zip3` VALUES('236', 'Norfolk', 'VA');
INSERT INTO `zip3` VALUES('237', 'Portsmouth', 'VA');
INSERT INTO `zip3` VALUES('238', 'Richmond', 'VA');
INSERT INTO `zip3` VALUES('239', 'Farmville', 'VA');
INSERT INTO `zip3` VALUES('240', 'Roanoke', 'VA');
INSERT INTO `zip3` VALUES('241', 'Roanoke', 'VA');
INSERT INTO `zip3` VALUES('242', 'Bristol', 'VA');
INSERT INTO `zip3` VALUES('243', 'Roanoke', 'VA');
INSERT INTO `zip3` VALUES('244', 'Charlottesvle', 'VA');
INSERT INTO `zip3` VALUES('245', 'Lynchburg', 'VA');
INSERT INTO `zip3` VALUES('246', 'Bluefield', 'WV');
INSERT INTO `zip3` VALUES('247', 'Bluefield', 'WV');
INSERT INTO `zip3` VALUES('248', 'Bluefield', 'WV');
INSERT INTO `zip3` VALUES('249', 'Lewisburg', 'WV');
INSERT INTO `zip3` VALUES('250', 'Charleston', 'WV');
INSERT INTO `zip3` VALUES('251', 'Charleston', 'WV');
INSERT INTO `zip3` VALUES('252', 'Charleston', 'WV');
INSERT INTO `zip3` VALUES('253', 'Charleston', 'WV');
INSERT INTO `zip3` VALUES('254', 'Martinsburg', 'WV');
INSERT INTO `zip3` VALUES('255', 'Huntington', 'WV');
INSERT INTO `zip3` VALUES('256', 'Huntington', 'WV');
INSERT INTO `zip3` VALUES('257', 'Huntington', 'WV');
INSERT INTO `zip3` VALUES('258', 'Beckley', 'WV');
INSERT INTO `zip3` VALUES('259', 'Beckley', 'WV');
INSERT INTO `zip3` VALUES('260', 'Wheeling', 'WV');
INSERT INTO `zip3` VALUES('261', 'Parkersburg', 'WV');
INSERT INTO `zip3` VALUES('262', 'Clarksburg', 'WV');
INSERT INTO `zip3` VALUES('263', 'Clarksburg', 'WV');
INSERT INTO `zip3` VALUES('264', 'Clarksburg', 'WV');
INSERT INTO `zip3` VALUES('265', 'Clarksburg', 'WV');
INSERT INTO `zip3` VALUES('266', 'Gassaway', 'WV');
INSERT INTO `zip3` VALUES('267', 'Cumberland', 'MD');
INSERT INTO `zip3` VALUES('268', 'Petersburg', 'WV');
INSERT INTO `zip3` VALUES('270', 'Greensboro', 'NC');
INSERT INTO `zip3` VALUES('271', 'Winston-Salem', 'NC');
INSERT INTO `zip3` VALUES('272', 'Greensboro', 'NC');
INSERT INTO `zip3` VALUES('273', 'Greensboro', 'NC');
INSERT INTO `zip3` VALUES('274', 'Greensboro', 'NC');
INSERT INTO `zip3` VALUES('275', 'Raleigh', 'NC');
INSERT INTO `zip3` VALUES('276', 'Raleigh', 'NC');
INSERT INTO `zip3` VALUES('277', 'Durham', 'NC');
INSERT INTO `zip3` VALUES('278', 'Rocky Mount', 'NC');
INSERT INTO `zip3` VALUES('279', 'Rocky Mount', 'NC');
INSERT INTO `zip3` VALUES('280', 'Charlotte', 'NC');
INSERT INTO `zip3` VALUES('281', 'Charlotte', 'NC');
INSERT INTO `zip3` VALUES('282', 'Charlotte', 'NC');
INSERT INTO `zip3` VALUES('283', 'Fayetteville', 'NC');
INSERT INTO `zip3` VALUES('284', 'Fayetteville', 'NC');
INSERT INTO `zip3` VALUES('285', 'Kinston', 'NC');
INSERT INTO `zip3` VALUES('286', 'Hickory', 'NC');
INSERT INTO `zip3` VALUES('287', 'Asheville', 'NC');
INSERT INTO `zip3` VALUES('288', 'Asheville', 'NC');
INSERT INTO `zip3` VALUES('289', 'Asheville', 'NC');
INSERT INTO `zip3` VALUES('290', 'Columbia', 'SC');
INSERT INTO `zip3` VALUES('291', 'Columbia', 'SC');
INSERT INTO `zip3` VALUES('292', 'Columbia', 'SC');
INSERT INTO `zip3` VALUES('293', 'Greenville', 'SC');
INSERT INTO `zip3` VALUES('294', 'Charleston', 'SC');
INSERT INTO `zip3` VALUES('295', 'Florence', 'SC');
INSERT INTO `zip3` VALUES('296', 'Greenville', 'SC');
INSERT INTO `zip3` VALUES('297', 'Charlotte', 'NC');
INSERT INTO `zip3` VALUES('298', 'Augusta', 'GA');
INSERT INTO `zip3` VALUES('299', 'Savannah', 'GA');
INSERT INTO `zip3` VALUES('300', 'North Metro', 'GA');
INSERT INTO `zip3` VALUES('301', 'North Metro', 'GA');
INSERT INTO `zip3` VALUES('302', 'Atlanta', 'GA');
INSERT INTO `zip3` VALUES('303', 'Atlanta', 'GA');
INSERT INTO `zip3` VALUES('304', 'Swainsboro', 'GA');
INSERT INTO `zip3` VALUES('305', 'Athens', 'GA');
INSERT INTO `zip3` VALUES('306', 'Athens', 'GA');
INSERT INTO `zip3` VALUES('307', 'Chattanooga', 'TN');
INSERT INTO `zip3` VALUES('308', 'Augusta', 'GA');
INSERT INTO `zip3` VALUES('309', 'Augusta', 'GA');
INSERT INTO `zip3` VALUES('310', 'Macon', 'GA');
INSERT INTO `zip3` VALUES('311', 'Atlanta', 'GA');
INSERT INTO `zip3` VALUES('312', 'Macon', 'GA');
INSERT INTO `zip3` VALUES('313', 'Savannah', 'GA');
INSERT INTO `zip3` VALUES('314', 'Savannah', 'GA');
INSERT INTO `zip3` VALUES('315', 'Waycross', 'GA');
INSERT INTO `zip3` VALUES('316', 'Valdosta', 'GA');
INSERT INTO `zip3` VALUES('317', 'Albany', 'GA');
INSERT INTO `zip3` VALUES('318', 'Columbus', 'GA');
INSERT INTO `zip3` VALUES('319', 'Columbus', 'GA');
INSERT INTO `zip3` VALUES('320', 'Jacksonville', 'FL');
INSERT INTO `zip3` VALUES('321', 'Daytona Beach', 'FL');
INSERT INTO `zip3` VALUES('322', 'Jacksonville', 'FL');
INSERT INTO `zip3` VALUES('323', 'Tallahassee', 'FL');
INSERT INTO `zip3` VALUES('324', 'Panama City', 'FL');
INSERT INTO `zip3` VALUES('325', 'Pensacola', 'FL');
INSERT INTO `zip3` VALUES('326', 'Gainesville', 'FL');
INSERT INTO `zip3` VALUES('327', 'Mid-Florida', 'FL');
INSERT INTO `zip3` VALUES('328', 'Orlando', 'FL');
INSERT INTO `zip3` VALUES('329', 'Orlando', 'FL');
INSERT INTO `zip3` VALUES('330', 'South Florida', 'FL');
INSERT INTO `zip3` VALUES('331', 'Miami', 'FL');
INSERT INTO `zip3` VALUES('332', 'Miami', 'FL');
INSERT INTO `zip3` VALUES('333', 'Ft Lauderdale', 'FL');
INSERT INTO `zip3` VALUES('334', 'West Palm Bch', 'FL');
INSERT INTO `zip3` VALUES('335', 'Tampa', 'FL');
INSERT INTO `zip3` VALUES('336', 'Tampa', 'FL');
INSERT INTO `zip3` VALUES('337', 'St Petersburg', 'FL');
INSERT INTO `zip3` VALUES('338', 'Lakeland', 'FL');
INSERT INTO `zip3` VALUES('339', 'Ft Myers', 'FL');
INSERT INTO `zip3` VALUES('340', 'Apo/Fpo', 'AA');
INSERT INTO `zip3` VALUES('341', 'Ft Myers', 'FL');
INSERT INTO `zip3` VALUES('342', 'Manasota', 'FL');
INSERT INTO `zip3` VALUES('344', 'Gainesville', 'FL');
INSERT INTO `zip3` VALUES('346', 'Tampa', 'FL');
INSERT INTO `zip3` VALUES('347', 'Orlando', 'FL');
INSERT INTO `zip3` VALUES('349', 'West Palm Bch', 'FL');
INSERT INTO `zip3` VALUES('350', 'Birmingham', 'AL');
INSERT INTO `zip3` VALUES('351', 'Birmingham', 'AL');
INSERT INTO `zip3` VALUES('352', 'Birmingham', 'AL');
INSERT INTO `zip3` VALUES('354', 'Tuscaloosa', 'AL');
INSERT INTO `zip3` VALUES('355', 'Birmingham', 'AL');
INSERT INTO `zip3` VALUES('356', 'Huntsville', 'AL');
INSERT INTO `zip3` VALUES('357', 'Huntsville', 'AL');
INSERT INTO `zip3` VALUES('358', 'Huntsville', 'AL');
INSERT INTO `zip3` VALUES('359', 'Birmingham', 'AL');
INSERT INTO `zip3` VALUES('360', 'Montgomery', 'AL');
INSERT INTO `zip3` VALUES('361', 'Montgomery', 'AL');
INSERT INTO `zip3` VALUES('362', 'Anniston', 'AL');
INSERT INTO `zip3` VALUES('363', 'Dothan', 'AL');
INSERT INTO `zip3` VALUES('364', 'Evergreen', 'AL');
INSERT INTO `zip3` VALUES('365', 'Mobile', 'AL');
INSERT INTO `zip3` VALUES('366', 'Mobile', 'AL');
INSERT INTO `zip3` VALUES('367', 'Montgomery', 'AL');
INSERT INTO `zip3` VALUES('368', 'Montgomery', 'AL');
INSERT INTO `zip3` VALUES('369', 'Meridian', 'MS');
INSERT INTO `zip3` VALUES('370', 'Nashville', 'TN');
INSERT INTO `zip3` VALUES('371', 'Nashville', 'TN');
INSERT INTO `zip3` VALUES('372', 'Nashville', 'TN');
INSERT INTO `zip3` VALUES('373', 'Chattanooga', 'TN');
INSERT INTO `zip3` VALUES('374', 'Chattanooga', 'TN');
INSERT INTO `zip3` VALUES('375', 'Memphis', 'TN');
INSERT INTO `zip3` VALUES('376', 'Johnson City', 'TN');
INSERT INTO `zip3` VALUES('377', 'Knoxville', 'TN');
INSERT INTO `zip3` VALUES('378', 'Knoxville', 'TN');
INSERT INTO `zip3` VALUES('379', 'Knoxville', 'TN');
INSERT INTO `zip3` VALUES('380', 'Memphis', 'TN');
INSERT INTO `zip3` VALUES('381', 'Memphis', 'TN');
INSERT INTO `zip3` VALUES('382', 'Mckenzie', 'TN');
INSERT INTO `zip3` VALUES('383', 'Jackson', 'TN');
INSERT INTO `zip3` VALUES('384', 'Columbia', 'TN');
INSERT INTO `zip3` VALUES('385', 'Cookeville', 'TN');
INSERT INTO `zip3` VALUES('386', 'Memphis', 'TN');
INSERT INTO `zip3` VALUES('387', 'Greenville', 'MS');
INSERT INTO `zip3` VALUES('388', 'Tupelo', 'MS');
INSERT INTO `zip3` VALUES('389', 'Grenada', 'MS');
INSERT INTO `zip3` VALUES('390', 'Jackson', 'MS');
INSERT INTO `zip3` VALUES('391', 'Jackson', 'MS');
INSERT INTO `zip3` VALUES('392', 'Jackson', 'MS');
INSERT INTO `zip3` VALUES('393', 'Meridian', 'MS');
INSERT INTO `zip3` VALUES('394', 'Hattiesburg', 'MS');
INSERT INTO `zip3` VALUES('395', 'Gulfport', 'MS');
INSERT INTO `zip3` VALUES('396', 'Mccomb', 'MS');
INSERT INTO `zip3` VALUES('397', 'Columbus', 'MS');
INSERT INTO `zip3` VALUES('398', 'Albany', 'GA');
INSERT INTO `zip3` VALUES('399', 'Atlanta', 'GA');
INSERT INTO `zip3` VALUES('400', 'Louisville', 'KY');
INSERT INTO `zip3` VALUES('401', 'Louisville', 'KY');
INSERT INTO `zip3` VALUES('402', 'Louisville', 'KY');
INSERT INTO `zip3` VALUES('403', 'Lexington', 'KY');
INSERT INTO `zip3` VALUES('404', 'Lexington', 'KY');
INSERT INTO `zip3` VALUES('405', 'Lexington', 'KY');
INSERT INTO `zip3` VALUES('406', 'Frankfort', 'KY');
INSERT INTO `zip3` VALUES('407', 'London', 'KY');
INSERT INTO `zip3` VALUES('408', 'London', 'KY');
INSERT INTO `zip3` VALUES('409', 'London', 'KY');
INSERT INTO `zip3` VALUES('410', 'Cincinnati', 'OH');
INSERT INTO `zip3` VALUES('411', 'Ashland', 'KY');
INSERT INTO `zip3` VALUES('412', 'Ashland', 'KY');
INSERT INTO `zip3` VALUES('413', 'Campton', 'KY');
INSERT INTO `zip3` VALUES('414', 'Campton', 'KY');
INSERT INTO `zip3` VALUES('415', 'Pikeville', 'KY');
INSERT INTO `zip3` VALUES('416', 'Pikeville', 'KY');
INSERT INTO `zip3` VALUES('417', 'Hazard', 'KY');
INSERT INTO `zip3` VALUES('418', 'Hazard', 'KY');
INSERT INTO `zip3` VALUES('420', 'Paducah', 'KY');
INSERT INTO `zip3` VALUES('421', 'Bowling Green', 'KY');
INSERT INTO `zip3` VALUES('422', 'Bowling Green', 'KY');
INSERT INTO `zip3` VALUES('423', 'Owensboro', 'KY');
INSERT INTO `zip3` VALUES('424', 'Evansville', 'IN');
INSERT INTO `zip3` VALUES('425', 'Somerset', 'KY');
INSERT INTO `zip3` VALUES('426', 'Somerset', 'KY');
INSERT INTO `zip3` VALUES('427', 'Elizabethtown', 'KY');
INSERT INTO `zip3` VALUES('430', 'Columbus', 'OH');
INSERT INTO `zip3` VALUES('431', 'Columbus', 'OH');
INSERT INTO `zip3` VALUES('432', 'Columbus', 'OH');
INSERT INTO `zip3` VALUES('433', 'Columbus', 'OH');
INSERT INTO `zip3` VALUES('434', 'Toledo', 'OH');
INSERT INTO `zip3` VALUES('435', 'Toledo', 'OH');
INSERT INTO `zip3` VALUES('436', 'Toledo', 'OH');
INSERT INTO `zip3` VALUES('437', 'Zanesville', 'OH');
INSERT INTO `zip3` VALUES('438', 'Zanesville', 'OH');
INSERT INTO `zip3` VALUES('439', 'Steubenville', 'OH');
INSERT INTO `zip3` VALUES('440', 'Cleveland', 'OH');
INSERT INTO `zip3` VALUES('441', 'Cleveland', 'OH');
INSERT INTO `zip3` VALUES('442', 'Akron', 'OH');
INSERT INTO `zip3` VALUES('443', 'Akron', 'OH');
INSERT INTO `zip3` VALUES('444', 'Youngstown', 'OH');
INSERT INTO `zip3` VALUES('445', 'Youngstown', 'OH');
INSERT INTO `zip3` VALUES('446', 'Canton', 'OH');
INSERT INTO `zip3` VALUES('447', 'Canton', 'OH');
INSERT INTO `zip3` VALUES('448', 'Mansfield', 'OH');
INSERT INTO `zip3` VALUES('449', 'Mansfield', 'OH');
INSERT INTO `zip3` VALUES('450', 'Cincinnati', 'OH');
INSERT INTO `zip3` VALUES('451', 'Cincinnati', 'OH');
INSERT INTO `zip3` VALUES('452', 'Cincinnati', 'OH');
INSERT INTO `zip3` VALUES('453', 'Dayton', 'OH');
INSERT INTO `zip3` VALUES('454', 'Dayton', 'OH');
INSERT INTO `zip3` VALUES('455', 'Springfield', 'OH');
INSERT INTO `zip3` VALUES('456', 'Chillicothe', 'OH');
INSERT INTO `zip3` VALUES('457', 'Athens', 'OH');
INSERT INTO `zip3` VALUES('458', 'Lima', 'OH');
INSERT INTO `zip3` VALUES('459', 'Cincinnati', 'OH');
INSERT INTO `zip3` VALUES('460', 'Indianapolis', 'IN');
INSERT INTO `zip3` VALUES('461', 'Indianapolis', 'IN');
INSERT INTO `zip3` VALUES('462', 'Indianapolis', 'IN');
INSERT INTO `zip3` VALUES('463', 'Gary', 'IN');
INSERT INTO `zip3` VALUES('464', 'Gary', 'IN');
INSERT INTO `zip3` VALUES('465', 'South Bend', 'IN');
INSERT INTO `zip3` VALUES('466', 'South Bend', 'IN');
INSERT INTO `zip3` VALUES('467', 'Fort Wayne', 'IN');
INSERT INTO `zip3` VALUES('468', 'Fort Wayne', 'IN');
INSERT INTO `zip3` VALUES('469', 'Kokomo', 'IN');
INSERT INTO `zip3` VALUES('470', 'Cincinnati', 'OH');
INSERT INTO `zip3` VALUES('471', 'Louisville', 'KY');
INSERT INTO `zip3` VALUES('472', 'Columbus', 'IN');
INSERT INTO `zip3` VALUES('473', 'Muncie', 'IN');
INSERT INTO `zip3` VALUES('474', 'Bloomington', 'IN');
INSERT INTO `zip3` VALUES('475', 'Terre Haute', 'IN');
INSERT INTO `zip3` VALUES('476', 'Evansville', 'IN');
INSERT INTO `zip3` VALUES('477', 'Evansville', 'IN');
INSERT INTO `zip3` VALUES('478', 'Terre Haute', 'IN');
INSERT INTO `zip3` VALUES('479', 'Lafayette', 'IN');
INSERT INTO `zip3` VALUES('480', 'Royal Oak', 'MI');
INSERT INTO `zip3` VALUES('481', 'Detroit', 'MI');
INSERT INTO `zip3` VALUES('482', 'Detroit', 'MI');
INSERT INTO `zip3` VALUES('483', 'Royal Oak', 'MI');
INSERT INTO `zip3` VALUES('484', 'Flint', 'MI');
INSERT INTO `zip3` VALUES('485', 'Flint', 'MI');
INSERT INTO `zip3` VALUES('486', 'Saginaw', 'MI');
INSERT INTO `zip3` VALUES('487', 'Saginaw', 'MI');
INSERT INTO `zip3` VALUES('488', 'Lansing', 'MI');
INSERT INTO `zip3` VALUES('489', 'Lansing', 'MI');
INSERT INTO `zip3` VALUES('490', 'Kalamazoo', 'MI');
INSERT INTO `zip3` VALUES('491', 'Kalamazoo', 'MI');
INSERT INTO `zip3` VALUES('492', 'Jackson', 'MI');
INSERT INTO `zip3` VALUES('493', 'Grand Rapids', 'MI');
INSERT INTO `zip3` VALUES('494', 'Grand Rapids', 'MI');
INSERT INTO `zip3` VALUES('495', 'Grand Rapids', 'MI');
INSERT INTO `zip3` VALUES('496', 'Traverse City', 'MI');
INSERT INTO `zip3` VALUES('497', 'Gaylord', 'MI');
INSERT INTO `zip3` VALUES('498', 'Iron Mountain', 'MI');
INSERT INTO `zip3` VALUES('499', 'Iron Mountain', 'MI');
INSERT INTO `zip3` VALUES('500', 'Des Moines', 'IA');
INSERT INTO `zip3` VALUES('501', 'Des Moines', 'IA');
INSERT INTO `zip3` VALUES('502', 'Des Moines', 'IA');
INSERT INTO `zip3` VALUES('503', 'Des Moines', 'IA');
INSERT INTO `zip3` VALUES('504', 'Waterloo', 'IA');
INSERT INTO `zip3` VALUES('505', 'Fort Dodge', 'IA');
INSERT INTO `zip3` VALUES('506', 'Waterloo', 'IA');
INSERT INTO `zip3` VALUES('507', 'Waterloo', 'IA');
INSERT INTO `zip3` VALUES('508', 'Creston', 'IA');
INSERT INTO `zip3` VALUES('509', 'Des Moines', 'IA');
INSERT INTO `zip3` VALUES('510', 'Sioux City', 'IA');
INSERT INTO `zip3` VALUES('511', 'Sioux City', 'IA');
INSERT INTO `zip3` VALUES('512', 'Sioux City', 'IA');
INSERT INTO `zip3` VALUES('513', 'Sioux City', 'IA');
INSERT INTO `zip3` VALUES('514', 'Carroll', 'IA');
INSERT INTO `zip3` VALUES('515', 'Omaha', 'NE');
INSERT INTO `zip3` VALUES('516', 'Omaha', 'NE');
INSERT INTO `zip3` VALUES('520', 'Dubuque', 'IA');
INSERT INTO `zip3` VALUES('521', 'Decorah', 'IA');
INSERT INTO `zip3` VALUES('522', 'Cedar Rapids', 'IA');
INSERT INTO `zip3` VALUES('523', 'Cedar Rapids', 'IA');
INSERT INTO `zip3` VALUES('524', 'Cedar Rapids', 'IA');
INSERT INTO `zip3` VALUES('525', 'Des Moines', 'IA');
INSERT INTO `zip3` VALUES('526', 'Burlington', 'IA');
INSERT INTO `zip3` VALUES('527', 'Quad Cities', 'IL');
INSERT INTO `zip3` VALUES('528', 'Davenport', 'IA');
INSERT INTO `zip3` VALUES('530', 'Milwaukee', 'WI');
INSERT INTO `zip3` VALUES('531', 'Milwaukee', 'WI');
INSERT INTO `zip3` VALUES('532', 'Milwaukee', 'WI');
INSERT INTO `zip3` VALUES('534', 'Racine', 'WI');
INSERT INTO `zip3` VALUES('535', 'Madison', 'WI');
INSERT INTO `zip3` VALUES('537', 'Madison', 'WI');
INSERT INTO `zip3` VALUES('538', 'Madison', 'WI');
INSERT INTO `zip3` VALUES('539', 'Portage', 'WI');
INSERT INTO `zip3` VALUES('540', 'St Paul', 'MN');
INSERT INTO `zip3` VALUES('541', 'Green Bay', 'WI');
INSERT INTO `zip3` VALUES('542', 'Green Bay', 'WI');
INSERT INTO `zip3` VALUES('543', 'Green Bay', 'WI');
INSERT INTO `zip3` VALUES('544', 'Wausau', 'WI');
INSERT INTO `zip3` VALUES('545', 'Rhinelander', 'WI');
INSERT INTO `zip3` VALUES('546', 'La Crosse', 'WI');
INSERT INTO `zip3` VALUES('547', 'Eau Claire', 'WI');
INSERT INTO `zip3` VALUES('548', 'Spooner', 'WI');
INSERT INTO `zip3` VALUES('549', 'Oshkosh', 'WI');
INSERT INTO `zip3` VALUES('550', 'St Paul', 'MN');
INSERT INTO `zip3` VALUES('551', 'St Paul', 'MN');
INSERT INTO `zip3` VALUES('553', 'Minneapolis', 'MN');
INSERT INTO `zip3` VALUES('554', 'Minneapolis', 'MN');
INSERT INTO `zip3` VALUES('555', 'Minneapolis', 'MN');
INSERT INTO `zip3` VALUES('556', 'Duluth', 'MN');
INSERT INTO `zip3` VALUES('557', 'Duluth', 'MN');
INSERT INTO `zip3` VALUES('558', 'Duluth', 'MN');
INSERT INTO `zip3` VALUES('559', 'Rochester', 'MN');
INSERT INTO `zip3` VALUES('560', 'Mankato', 'MN');
INSERT INTO `zip3` VALUES('561', 'Mankato', 'MN');
INSERT INTO `zip3` VALUES('562', 'Willmar', 'MN');
INSERT INTO `zip3` VALUES('563', 'St Cloud', 'MN');
INSERT INTO `zip3` VALUES('564', 'Brainerd', 'MN');
INSERT INTO `zip3` VALUES('565', 'Detroit Lakes', 'MN');
INSERT INTO `zip3` VALUES('566', 'Bemidji', 'MN');
INSERT INTO `zip3` VALUES('567', 'Grand Forks', 'ND');
INSERT INTO `zip3` VALUES('570', 'Sioux Falls', 'SD');
INSERT INTO `zip3` VALUES('571', 'Sioux Falls', 'SD');
INSERT INTO `zip3` VALUES('572', 'Dakota Central', 'SD');
INSERT INTO `zip3` VALUES('573', 'Dakota Central', 'SD');
INSERT INTO `zip3` VALUES('574', 'Aberdeen', 'SD');
INSERT INTO `zip3` VALUES('575', 'Pierre', 'SD');
INSERT INTO `zip3` VALUES('576', 'Mobridge', 'SD');
INSERT INTO `zip3` VALUES('577', 'Rapid City', 'SD');
INSERT INTO `zip3` VALUES('580', 'Fargo', 'ND');
INSERT INTO `zip3` VALUES('581', 'Fargo', 'ND');
INSERT INTO `zip3` VALUES('582', 'Grand Forks', 'ND');
INSERT INTO `zip3` VALUES('583', 'Devils Lake', 'ND');
INSERT INTO `zip3` VALUES('584', 'Jamestown', 'ND');
INSERT INTO `zip3` VALUES('585', 'Bismarck', 'ND');
INSERT INTO `zip3` VALUES('586', 'Bismarck', 'ND');
INSERT INTO `zip3` VALUES('587', 'Minot', 'ND');
INSERT INTO `zip3` VALUES('588', 'Williston', 'ND');
INSERT INTO `zip3` VALUES('590', 'Billings', 'MT');
INSERT INTO `zip3` VALUES('591', 'Billings', 'MT');
INSERT INTO `zip3` VALUES('592', 'Wolf Point', 'MT');
INSERT INTO `zip3` VALUES('593', 'Miles City', 'MT');
INSERT INTO `zip3` VALUES('594', 'Great Falls', 'MT');
INSERT INTO `zip3` VALUES('595', 'Havre', 'MT');
INSERT INTO `zip3` VALUES('596', 'Helena', 'MT');
INSERT INTO `zip3` VALUES('597', 'Butte', 'MT');
INSERT INTO `zip3` VALUES('598', 'Missoula', 'MT');
INSERT INTO `zip3` VALUES('599', 'Kalispell', 'MT');
INSERT INTO `zip3` VALUES('600', 'Palatine', 'IL');
INSERT INTO `zip3` VALUES('601', 'Carol Stream', 'IL');
INSERT INTO `zip3` VALUES('602', 'Evanston', 'IL');
INSERT INTO `zip3` VALUES('603', 'Oak Park', 'IL');
INSERT INTO `zip3` VALUES('604', 'S Suburban', 'IL');
INSERT INTO `zip3` VALUES('605', 'Fox Valley', 'IL');
INSERT INTO `zip3` VALUES('606', 'Chicago', 'IL');
INSERT INTO `zip3` VALUES('607', 'Chicago', 'IL');
INSERT INTO `zip3` VALUES('608', 'Chicago', 'IL');
INSERT INTO `zip3` VALUES('609', 'Kankakee', 'IL');
INSERT INTO `zip3` VALUES('610', 'Rockford', 'IL');
INSERT INTO `zip3` VALUES('611', 'Rockford', 'IL');
INSERT INTO `zip3` VALUES('612', 'Quad Cities', 'IL');
INSERT INTO `zip3` VALUES('613', 'La Salle', 'IL');
INSERT INTO `zip3` VALUES('614', 'Galesburg', 'IL');
INSERT INTO `zip3` VALUES('615', 'Peoria', 'IL');
INSERT INTO `zip3` VALUES('616', 'Peoria', 'IL');
INSERT INTO `zip3` VALUES('617', 'Bloomington', 'IL');
INSERT INTO `zip3` VALUES('618', 'Champaign', 'IL');
INSERT INTO `zip3` VALUES('619', 'Champaign', 'IL');
INSERT INTO `zip3` VALUES('620', 'St Louis', 'MO');
INSERT INTO `zip3` VALUES('622', 'St Louis', 'MO');
INSERT INTO `zip3` VALUES('623', 'Quincy', 'IL');
INSERT INTO `zip3` VALUES('624', 'Effingham', 'IL');
INSERT INTO `zip3` VALUES('625', 'Springfield', 'IL');
INSERT INTO `zip3` VALUES('626', 'Springfield', 'IL');
INSERT INTO `zip3` VALUES('627', 'Springfield', 'IL');
INSERT INTO `zip3` VALUES('628', 'Centralia', 'IL');
INSERT INTO `zip3` VALUES('629', 'Carbondale', 'IL');
INSERT INTO `zip3` VALUES('630', 'St Louis', 'MO');
INSERT INTO `zip3` VALUES('631', 'St Louis', 'MO');
INSERT INTO `zip3` VALUES('633', 'St Louis', 'MO');
INSERT INTO `zip3` VALUES('634', 'Quincy', 'IL');
INSERT INTO `zip3` VALUES('635', 'Quincy', 'IL');
INSERT INTO `zip3` VALUES('636', 'Cape Girardeau', 'MO');
INSERT INTO `zip3` VALUES('637', 'Cape Girardeau', 'MO');
INSERT INTO `zip3` VALUES('638', 'Cape Girardeau', 'MO');
INSERT INTO `zip3` VALUES('639', 'Cape Girardeau', 'MO');
INSERT INTO `zip3` VALUES('640', 'Kansas City', 'MO');
INSERT INTO `zip3` VALUES('641', 'Kansas City', 'MO');
INSERT INTO `zip3` VALUES('644', 'St Joseph', 'MO');
INSERT INTO `zip3` VALUES('645', 'St Joseph', 'MO');
INSERT INTO `zip3` VALUES('646', 'Chillicothe', 'MO');
INSERT INTO `zip3` VALUES('647', 'Harrisonville', 'MO');
INSERT INTO `zip3` VALUES('648', 'Springfield', 'MO');
INSERT INTO `zip3` VALUES('649', 'Kansas City', 'MO');
INSERT INTO `zip3` VALUES('650', 'Mid-Missouri', 'MO');
INSERT INTO `zip3` VALUES('651', 'Mid-Missouri', 'MO');
INSERT INTO `zip3` VALUES('652', 'Mid-Missouri', 'MO');
INSERT INTO `zip3` VALUES('653', 'Mid-Missouri', 'MO');
INSERT INTO `zip3` VALUES('654', 'Springfield', 'MO');
INSERT INTO `zip3` VALUES('655', 'Springfield', 'MO');
INSERT INTO `zip3` VALUES('656', 'Springfield', 'MO');
INSERT INTO `zip3` VALUES('657', 'Springfield', 'MO');
INSERT INTO `zip3` VALUES('658', 'Springfield', 'MO');
INSERT INTO `zip3` VALUES('660', 'Kansas City', 'KS');
INSERT INTO `zip3` VALUES('661', 'Kansas City', 'KS');
INSERT INTO `zip3` VALUES('662', 'Kansas City', 'KS');
INSERT INTO `zip3` VALUES('664', 'Topeka', 'KS');
INSERT INTO `zip3` VALUES('665', 'Topeka', 'KS');
INSERT INTO `zip3` VALUES('666', 'Topeka', 'KS');
INSERT INTO `zip3` VALUES('667', 'Ft Scott', 'KS');
INSERT INTO `zip3` VALUES('668', 'Topeka', 'KS');
INSERT INTO `zip3` VALUES('669', 'Salina', 'KS');
INSERT INTO `zip3` VALUES('670', 'Wichita', 'KS');
INSERT INTO `zip3` VALUES('671', 'Wichita', 'KS');
INSERT INTO `zip3` VALUES('672', 'Wichita', 'KS');
INSERT INTO `zip3` VALUES('673', 'Independence', 'KS');
INSERT INTO `zip3` VALUES('674', 'Salina', 'KS');
INSERT INTO `zip3` VALUES('675', 'Hutchinson', 'KS');
INSERT INTO `zip3` VALUES('676', 'Hays', 'KS');
INSERT INTO `zip3` VALUES('677', 'Colby', 'KS');
INSERT INTO `zip3` VALUES('678', 'Dodge City', 'KS');
INSERT INTO `zip3` VALUES('679', 'Liberal', 'KS');
INSERT INTO `zip3` VALUES('680', 'Omaha', 'NE');
INSERT INTO `zip3` VALUES('681', 'Omaha', 'NE');
INSERT INTO `zip3` VALUES('683', 'Lincoln', 'NE');
INSERT INTO `zip3` VALUES('684', 'Lincoln', 'NE');
INSERT INTO `zip3` VALUES('685', 'Lincoln', 'NE');
INSERT INTO `zip3` VALUES('686', 'Norfolk', 'NE');
INSERT INTO `zip3` VALUES('687', 'Norfolk', 'NE');
INSERT INTO `zip3` VALUES('688', 'Grand Island', 'NE');
INSERT INTO `zip3` VALUES('689', 'Grand Island', 'NE');
INSERT INTO `zip3` VALUES('690', 'Mc Cook', 'NE');
INSERT INTO `zip3` VALUES('691', 'North Platte', 'NE');
INSERT INTO `zip3` VALUES('692', 'Valentine', 'NE');
INSERT INTO `zip3` VALUES('693', 'Alliance', 'NE');
INSERT INTO `zip3` VALUES('700', 'New Orleans', 'LA');
INSERT INTO `zip3` VALUES('701', 'New Orleans', 'LA');
INSERT INTO `zip3` VALUES('703', 'Houma', 'LA');
INSERT INTO `zip3` VALUES('704', 'Mandeville', 'LA');
INSERT INTO `zip3` VALUES('705', 'Lafayette', 'LA');
INSERT INTO `zip3` VALUES('706', 'Lake Charles', 'LA');
INSERT INTO `zip3` VALUES('707', 'Baton Rouge', 'LA');
INSERT INTO `zip3` VALUES('708', 'Baton Rouge', 'LA');
INSERT INTO `zip3` VALUES('710', 'Shreveport', 'LA');
INSERT INTO `zip3` VALUES('711', 'Shreveport', 'LA');
INSERT INTO `zip3` VALUES('712', 'Monroe', 'LA');
INSERT INTO `zip3` VALUES('713', 'Alexandria', 'LA');
INSERT INTO `zip3` VALUES('714', 'Alexandria', 'LA');
INSERT INTO `zip3` VALUES('716', 'Pine Bluff', 'AR');
INSERT INTO `zip3` VALUES('717', 'Camden', 'AR');
INSERT INTO `zip3` VALUES('718', 'Texarkana', 'AR');
INSERT INTO `zip3` VALUES('719', 'Hot Springs Ntl Pk', 'AR');
INSERT INTO `zip3` VALUES('720', 'Little Rock', 'AR');
INSERT INTO `zip3` VALUES('721', 'Little Rock', 'AR');
INSERT INTO `zip3` VALUES('722', 'Little Rock', 'AR');
INSERT INTO `zip3` VALUES('723', 'Memphis', 'TN');
INSERT INTO `zip3` VALUES('724', 'Ne Arkansas', 'AR');
INSERT INTO `zip3` VALUES('725', 'Batesville', 'AR');
INSERT INTO `zip3` VALUES('726', 'Harrison', 'AR');
INSERT INTO `zip3` VALUES('727', 'Nw Arkansas', 'AR');
INSERT INTO `zip3` VALUES('728', 'Russellville', 'AR');
INSERT INTO `zip3` VALUES('729', 'Fort Smith', 'AR');
INSERT INTO `zip3` VALUES('730', 'Oklahoma City', 'OK');
INSERT INTO `zip3` VALUES('731', 'Oklahoma City', 'OK');
INSERT INTO `zip3` VALUES('733', 'Austin', 'TX');
INSERT INTO `zip3` VALUES('734', 'Ardmore', 'OK');
INSERT INTO `zip3` VALUES('735', 'Lawton', 'OK');
INSERT INTO `zip3` VALUES('736', 'Clinton', 'OK');
INSERT INTO `zip3` VALUES('737', 'Enid', 'OK');
INSERT INTO `zip3` VALUES('738', 'Woodward', 'OK');
INSERT INTO `zip3` VALUES('739', 'Liberal', 'KS');
INSERT INTO `zip3` VALUES('740', 'Tulsa', 'OK');
INSERT INTO `zip3` VALUES('741', 'Tulsa', 'OK');
INSERT INTO `zip3` VALUES('743', 'Tulsa', 'OK');
INSERT INTO `zip3` VALUES('744', 'Muskogee', 'OK');
INSERT INTO `zip3` VALUES('745', 'Mcalester', 'OK');
INSERT INTO `zip3` VALUES('746', 'Ponca City', 'OK');
INSERT INTO `zip3` VALUES('747', 'Durant', 'OK');
INSERT INTO `zip3` VALUES('748', 'Shawnee', 'OK');
INSERT INTO `zip3` VALUES('749', 'Poteau', 'OK');
INSERT INTO `zip3` VALUES('750', 'North Texas', 'TX');
INSERT INTO `zip3` VALUES('751', 'Dallas', 'TX');
INSERT INTO `zip3` VALUES('752', 'Dallas', 'TX');
INSERT INTO `zip3` VALUES('753', 'Dallas', 'TX');
INSERT INTO `zip3` VALUES('754', 'Greenville', 'TX');
INSERT INTO `zip3` VALUES('755', 'Texarkana', 'TX');
INSERT INTO `zip3` VALUES('756', 'East Texas', 'TX');
INSERT INTO `zip3` VALUES('757', 'East Texas', 'TX');
INSERT INTO `zip3` VALUES('758', 'Palestine', 'TX');
INSERT INTO `zip3` VALUES('759', 'Lufkin', 'TX');
INSERT INTO `zip3` VALUES('760', 'Ft Worth', 'TX');
INSERT INTO `zip3` VALUES('761', 'Ft Worth', 'TX');
INSERT INTO `zip3` VALUES('762', 'Ft Worth', 'TX');
INSERT INTO `zip3` VALUES('763', 'Wichita Falls', 'TX');
INSERT INTO `zip3` VALUES('764', 'Ft Worth', 'TX');
INSERT INTO `zip3` VALUES('765', 'Waco', 'TX');
INSERT INTO `zip3` VALUES('766', 'Waco', 'TX');
INSERT INTO `zip3` VALUES('767', 'Waco', 'TX');
INSERT INTO `zip3` VALUES('768', 'Abilene', 'TX');
INSERT INTO `zip3` VALUES('769', 'Midland', 'TX');
INSERT INTO `zip3` VALUES('770', 'Houston', 'TX');
INSERT INTO `zip3` VALUES('771', 'Houston', 'TX');
INSERT INTO `zip3` VALUES('772', 'Houston', 'TX');
INSERT INTO `zip3` VALUES('773', 'North Houston', 'TX');
INSERT INTO `zip3` VALUES('774', 'North Houston', 'TX');
INSERT INTO `zip3` VALUES('775', 'North Houston', 'TX');
INSERT INTO `zip3` VALUES('776', 'Beaumont', 'TX');
INSERT INTO `zip3` VALUES('777', 'Beaumont', 'TX');
INSERT INTO `zip3` VALUES('778', 'Bryan', 'TX');
INSERT INTO `zip3` VALUES('779', 'Victoria', 'TX');
INSERT INTO `zip3` VALUES('780', 'San Antonio', 'TX');
INSERT INTO `zip3` VALUES('781', 'San Antonio', 'TX');
INSERT INTO `zip3` VALUES('782', 'San Antonio', 'TX');
INSERT INTO `zip3` VALUES('783', 'Corpus Christi', 'TX');
INSERT INTO `zip3` VALUES('784', 'Corpus Christi', 'TX');
INSERT INTO `zip3` VALUES('785', 'Mcallen', 'TX');
INSERT INTO `zip3` VALUES('786', 'Austin', 'TX');
INSERT INTO `zip3` VALUES('787', 'Austin', 'TX');
INSERT INTO `zip3` VALUES('788', 'San Antonio', 'TX');
INSERT INTO `zip3` VALUES('789', 'Austin', 'TX');
INSERT INTO `zip3` VALUES('790', 'Amarillo', 'TX');
INSERT INTO `zip3` VALUES('791', 'Amarillo', 'TX');
INSERT INTO `zip3` VALUES('792', 'Childress', 'TX');
INSERT INTO `zip3` VALUES('793', 'Lubbock', 'TX');
INSERT INTO `zip3` VALUES('794', 'Lubbock', 'TX');
INSERT INTO `zip3` VALUES('795', 'Abilene', 'TX');
INSERT INTO `zip3` VALUES('796', 'Abilene', 'TX');
INSERT INTO `zip3` VALUES('797', 'Midland', 'TX');
INSERT INTO `zip3` VALUES('798', 'El Paso', 'TX');
INSERT INTO `zip3` VALUES('799', 'El Paso', 'TX');
INSERT INTO `zip3` VALUES('800', 'Denver', 'CO');
INSERT INTO `zip3` VALUES('801', 'Denver', 'CO');
INSERT INTO `zip3` VALUES('802', 'Denver', 'CO');
INSERT INTO `zip3` VALUES('803', 'Boulder', 'CO');
INSERT INTO `zip3` VALUES('804', 'Denver', 'CO');
INSERT INTO `zip3` VALUES('805', 'Longmont', 'CO');
INSERT INTO `zip3` VALUES('806', 'Denver', 'CO');
INSERT INTO `zip3` VALUES('807', 'Denver', 'CO');
INSERT INTO `zip3` VALUES('808', 'Colorado Spgs', 'CO');
INSERT INTO `zip3` VALUES('809', 'Colorado Spgs', 'CO');
INSERT INTO `zip3` VALUES('810', 'Colorado Spgs', 'CO');
INSERT INTO `zip3` VALUES('811', 'Alamosa', 'CO');
INSERT INTO `zip3` VALUES('812', 'Salida', 'CO');
INSERT INTO `zip3` VALUES('813', 'Durango', 'CO');
INSERT INTO `zip3` VALUES('814', 'Grand Junction', 'CO');
INSERT INTO `zip3` VALUES('815', 'Grand Junction', 'CO');
INSERT INTO `zip3` VALUES('816', 'Glenwood Springs', 'CO');
INSERT INTO `zip3` VALUES('820', 'Cheyenne', 'WY');
INSERT INTO `zip3` VALUES('821', 'Yellowstone Nl Pk', 'WY');
INSERT INTO `zip3` VALUES('822', 'Wheatland', 'WY');
INSERT INTO `zip3` VALUES('823', 'Rawlins', 'WY');
INSERT INTO `zip3` VALUES('824', 'Worland', 'WY');
INSERT INTO `zip3` VALUES('825', 'Riverton', 'WY');
INSERT INTO `zip3` VALUES('826', 'Casper', 'WY');
INSERT INTO `zip3` VALUES('827', 'Gillette', 'WY');
INSERT INTO `zip3` VALUES('828', 'Sheridan', 'WY');
INSERT INTO `zip3` VALUES('829', 'Rock Springs', 'WY');
INSERT INTO `zip3` VALUES('830', 'Rock Springs', 'WY');
INSERT INTO `zip3` VALUES('831', 'Rock Springs', 'WY');
INSERT INTO `zip3` VALUES('832', 'Pocatello', 'ID');
INSERT INTO `zip3` VALUES('833', 'Twin Falls', 'ID');
INSERT INTO `zip3` VALUES('834', 'Pocatello', 'ID');
INSERT INTO `zip3` VALUES('835', 'Lewiston', 'ID');
INSERT INTO `zip3` VALUES('836', 'Boise', 'ID');
INSERT INTO `zip3` VALUES('837', 'Boise', 'ID');
INSERT INTO `zip3` VALUES('838', 'Spokane', 'WA');
INSERT INTO `zip3` VALUES('840', 'Salt Lake Cty', 'UT');
INSERT INTO `zip3` VALUES('841', 'Salt Lake Cty', 'UT');
INSERT INTO `zip3` VALUES('842', 'Salt Lake Cty', 'UT');
INSERT INTO `zip3` VALUES('843', 'Salt Lake Cty', 'UT');
INSERT INTO `zip3` VALUES('844', 'Ogden', 'UT');
INSERT INTO `zip3` VALUES('845', 'Provo', 'UT');
INSERT INTO `zip3` VALUES('846', 'Provo', 'UT');
INSERT INTO `zip3` VALUES('847', 'Provo', 'UT');
INSERT INTO `zip3` VALUES('850', 'Phoenix', 'AZ');
INSERT INTO `zip3` VALUES('852', 'Phoenix', 'AZ');
INSERT INTO `zip3` VALUES('853', 'Phoenix', 'AZ');
INSERT INTO `zip3` VALUES('855', 'Globe', 'AZ');
INSERT INTO `zip3` VALUES('856', 'Tucson', 'AZ');
INSERT INTO `zip3` VALUES('857', 'Tucson', 'AZ');
INSERT INTO `zip3` VALUES('859', 'Show Low', 'AZ');
INSERT INTO `zip3` VALUES('860', 'Flagstaff', 'AZ');
INSERT INTO `zip3` VALUES('863', 'Prescott', 'AZ');
INSERT INTO `zip3` VALUES('864', 'Kingman', 'AZ');
INSERT INTO `zip3` VALUES('865', 'Gallup', 'NM');
INSERT INTO `zip3` VALUES('870', 'Albuquerque', 'NM');
INSERT INTO `zip3` VALUES('871', 'Albuquerque', 'NM');
INSERT INTO `zip3` VALUES('872', 'Albuquerque', 'NM');
INSERT INTO `zip3` VALUES('873', 'Gallup', 'NM');
INSERT INTO `zip3` VALUES('874', 'Farmington', 'NM');
INSERT INTO `zip3` VALUES('875', 'Albuquerque', 'NM');
INSERT INTO `zip3` VALUES('877', 'Las Vegas', 'NM');
INSERT INTO `zip3` VALUES('878', 'Socorro', 'NM');
INSERT INTO `zip3` VALUES('879', 'Truth Or Cons', 'NM');
INSERT INTO `zip3` VALUES('880', 'Las Cruces', 'NM');
INSERT INTO `zip3` VALUES('881', 'Clovis', 'NM');
INSERT INTO `zip3` VALUES('882', 'Roswell', 'NM');
INSERT INTO `zip3` VALUES('883', 'Alamogordo', 'NM');
INSERT INTO `zip3` VALUES('884', 'Tucumcari', 'NM');
INSERT INTO `zip3` VALUES('885', 'El Paso', 'TX');
INSERT INTO `zip3` VALUES('889', 'Las Vegas', 'NV');
INSERT INTO `zip3` VALUES('890', 'Las Vegas', 'NV');
INSERT INTO `zip3` VALUES('891', 'Las Vegas', 'NV');
INSERT INTO `zip3` VALUES('893', 'Ely', 'NV');
INSERT INTO `zip3` VALUES('894', 'Reno', 'NV');
INSERT INTO `zip3` VALUES('895', 'Reno', 'NV');
INSERT INTO `zip3` VALUES('897', 'Carson City', 'NV');
INSERT INTO `zip3` VALUES('898', 'Elko', 'NV');
INSERT INTO `zip3` VALUES('900', 'Los Angeles', 'CA');
INSERT INTO `zip3` VALUES('901', 'Los Angeles', 'CA');
INSERT INTO `zip3` VALUES('902', 'Inglewood', 'CA');
INSERT INTO `zip3` VALUES('903', 'Inglewood', 'CA');
INSERT INTO `zip3` VALUES('904', 'Santa Monica', 'CA');
INSERT INTO `zip3` VALUES('905', 'Torrance', 'CA');
INSERT INTO `zip3` VALUES('906', 'Long Beach', 'CA');
INSERT INTO `zip3` VALUES('907', 'Long Beach', 'CA');
INSERT INTO `zip3` VALUES('908', 'Long Beach', 'CA');
INSERT INTO `zip3` VALUES('910', 'Pasadena', 'CA');
INSERT INTO `zip3` VALUES('911', 'Pasadena', 'CA');
INSERT INTO `zip3` VALUES('912', 'Glendale', 'CA');
INSERT INTO `zip3` VALUES('913', 'Van Nuys', 'CA');
INSERT INTO `zip3` VALUES('914', 'Van Nuys', 'CA');
INSERT INTO `zip3` VALUES('915', 'Burbank', 'CA');
INSERT INTO `zip3` VALUES('916', 'North Hollywood', 'CA');
INSERT INTO `zip3` VALUES('917', 'Industry', 'CA');
INSERT INTO `zip3` VALUES('918', 'Industry', 'CA');
INSERT INTO `zip3` VALUES('919', 'San Diego', 'CA');
INSERT INTO `zip3` VALUES('920', 'San Diego', 'CA');
INSERT INTO `zip3` VALUES('921', 'San Diego', 'CA');
INSERT INTO `zip3` VALUES('922', 'Sn Bernardino', 'CA');
INSERT INTO `zip3` VALUES('923', 'Sn Bernardino', 'CA');
INSERT INTO `zip3` VALUES('924', 'Sn Bernardino', 'CA');
INSERT INTO `zip3` VALUES('925', 'Sn Bernardino', 'CA');
INSERT INTO `zip3` VALUES('926', 'Santa Ana', 'CA');
INSERT INTO `zip3` VALUES('927', 'Santa Ana', 'CA');
INSERT INTO `zip3` VALUES('928', 'Anaheim', 'CA');
INSERT INTO `zip3` VALUES('930', 'Oxnard', 'CA');
INSERT INTO `zip3` VALUES('931', 'Santa Barbara', 'CA');
INSERT INTO `zip3` VALUES('932', 'Bakersfield', 'CA');
INSERT INTO `zip3` VALUES('933', 'Bakersfield', 'CA');
INSERT INTO `zip3` VALUES('934', 'Santa Barbara', 'CA');
INSERT INTO `zip3` VALUES('935', 'Mojave', 'CA');
INSERT INTO `zip3` VALUES('936', 'Fresno', 'CA');
INSERT INTO `zip3` VALUES('937', 'Fresno', 'CA');
INSERT INTO `zip3` VALUES('938', 'Fresno', 'CA');
INSERT INTO `zip3` VALUES('939', 'Salinas', 'CA');
INSERT INTO `zip3` VALUES('940', 'San Francisco', 'CA');
INSERT INTO `zip3` VALUES('941', 'San Francisco', 'CA');
INSERT INTO `zip3` VALUES('942', 'Sacramento', 'CA');
INSERT INTO `zip3` VALUES('943', 'Palo Alto', 'CA');
INSERT INTO `zip3` VALUES('944', 'San Mateo', 'CA');
INSERT INTO `zip3` VALUES('945', 'Oakland', 'CA');
INSERT INTO `zip3` VALUES('946', 'Oakland', 'CA');
INSERT INTO `zip3` VALUES('947', 'Berkeley', 'CA');
INSERT INTO `zip3` VALUES('948', 'Richmond', 'CA');
INSERT INTO `zip3` VALUES('949', 'North Bay', 'CA');
INSERT INTO `zip3` VALUES('950', 'San Jose', 'CA');
INSERT INTO `zip3` VALUES('951', 'San Jose', 'CA');
INSERT INTO `zip3` VALUES('952', 'Stockton', 'CA');
INSERT INTO `zip3` VALUES('953', 'Stockton', 'CA');
INSERT INTO `zip3` VALUES('954', 'North Bay', 'CA');
INSERT INTO `zip3` VALUES('955', 'Eureka', 'CA');
INSERT INTO `zip3` VALUES('956', 'Sacramento', 'CA');
INSERT INTO `zip3` VALUES('957', 'Sacramento', 'CA');
INSERT INTO `zip3` VALUES('958', 'Sacramento', 'CA');
INSERT INTO `zip3` VALUES('959', 'Marysville', 'CA');
INSERT INTO `zip3` VALUES('960', 'Redding', 'CA');
INSERT INTO `zip3` VALUES('961', 'Reno', 'NV');
INSERT INTO `zip3` VALUES('962', 'Apo/Fpo', 'AP');
INSERT INTO `zip3` VALUES('963', 'Apo/Fpo', 'AP');
INSERT INTO `zip3` VALUES('964', 'Apo/Fpo', 'AP');
INSERT INTO `zip3` VALUES('965', 'Apo/Fpo', 'AP');
INSERT INTO `zip3` VALUES('966', 'Fpo', 'AP');
INSERT INTO `zip3` VALUES('967', 'Honolulu', 'HI');
INSERT INTO `zip3` VALUES('968', 'Honolulu', 'HI');
INSERT INTO `zip3` VALUES('969', 'Barrigada', 'GU');
INSERT INTO `zip3` VALUES('970', 'Portland', 'OR');
INSERT INTO `zip3` VALUES('971', 'Portland', 'OR');
INSERT INTO `zip3` VALUES('972', 'Portland', 'OR');
INSERT INTO `zip3` VALUES('973', 'Salem', 'OR');
INSERT INTO `zip3` VALUES('974', 'Eugene', 'OR');
INSERT INTO `zip3` VALUES('975', 'Medford', 'OR');
INSERT INTO `zip3` VALUES('976', 'Klamath Falls', 'OR');
INSERT INTO `zip3` VALUES('977', 'Bend', 'OR');
INSERT INTO `zip3` VALUES('978', 'Pendleton', 'OR');
INSERT INTO `zip3` VALUES('979', 'Boise', 'ID');
INSERT INTO `zip3` VALUES('980', 'Seattle', 'WA');
INSERT INTO `zip3` VALUES('981', 'Seattle', 'WA');
INSERT INTO `zip3` VALUES('982', 'Everett', 'WA');
INSERT INTO `zip3` VALUES('983', 'Tacoma', 'WA');
INSERT INTO `zip3` VALUES('984', 'Tacoma', 'WA');
INSERT INTO `zip3` VALUES('985', 'Olympia', 'WA');
INSERT INTO `zip3` VALUES('986', 'Portland', 'OR');
INSERT INTO `zip3` VALUES('988', 'Wenatchee', 'WA');
INSERT INTO `zip3` VALUES('989', 'Yakima', 'WA');
INSERT INTO `zip3` VALUES('990', 'Spokane', 'WA');
INSERT INTO `zip3` VALUES('991', 'Spokane', 'WA');
INSERT INTO `zip3` VALUES('992', 'Spokane', 'WA');
INSERT INTO `zip3` VALUES('993', 'Pasco', 'WA');
INSERT INTO `zip3` VALUES('994', 'Lewiston', 'ID');
INSERT INTO `zip3` VALUES('995', 'Anchorage', 'AK');
INSERT INTO `zip3` VALUES('996', 'Anchorage', 'AK');
INSERT INTO `zip3` VALUES('997', 'Fairbanks', 'AK');
INSERT INTO `zip3` VALUES('998', 'Juneau', 'AK');
INSERT INTO `zip3` VALUES('999', 'Ketchikan', 'AK');

-- --------------------------------------------------------

--
-- Table structure for table `zips`
--

DROP TABLE IF EXISTS `zips`;
CREATE TABLE IF NOT EXISTS `zips` (
  `zip` int(11) NOT NULL,
  `type` varchar(8) NOT NULL,
  `decommissioned` bit(1) NOT NULL,
  `primary_city` varchar(27) NOT NULL,
  `acceptable_cities` varchar(282) DEFAULT NULL,
  `unacceptable_cities` varchar(2208) DEFAULT NULL,
  `state` varchar(2) NOT NULL,
  `county` varchar(39) DEFAULT NULL,
  `timezone` varchar(30) DEFAULT NULL,
  `area_codes` varchar(35) DEFAULT NULL,
  `world_region` varchar(2) NOT NULL,
  `country` varchar(2) NOT NULL,
  `latitude` decimal(6,2) NOT NULL,
  `longitude` decimal(7,2) NOT NULL,
  `irs_estimated_population_2015` int(11) NOT NULL,
  PRIMARY KEY (`zip`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

--
-- Dumping data for table `zips`
--

INSERT INTO `zips` (`zip`, `type`, `decommissioned`, `primary_city`, `acceptable_cities`, `unacceptable_cities`, `state`, `county`, `timezone`, `area_codes`, `world_region`, `country`, `latitude`, `longitude`, `irs_estimated_population_2015`) VALUES
(1001, 'STANDARD', b'0', 'Agawam', NULL, NULL, 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.06, -72.61, 15220),
(1002, 'STANDARD', b'0', 'Amherst', 'Cushman, Pelham', 'South Amherst', 'MA', 'Hampshire County', 'America/New_York', '413', 'NA', 'US', 42.37, -72.52, 16570),
(1003, 'PO BOX', b'0', 'Amherst', NULL, NULL, 'MA', 'Hampshire County', 'America/New_York', '413', 'NA', 'US', 42.39, -72.52, 184),
(1004, 'PO BOX', b'0', 'Amherst', NULL, NULL, 'MA', 'Hampshire County', 'America/New_York', '413', 'NA', 'US', 42.37, -72.52, 794),
(1005, 'STANDARD', b'0', 'Barre', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '978', 'NA', 'US', 42.42, -72.10, 4270),
(1007, 'STANDARD', b'0', 'Belchertown', NULL, NULL, 'MA', 'Hampshire County', 'America/New_York', '413', 'NA', 'US', 42.27, -72.40, 14180),
(1008, 'STANDARD', b'0', 'Blandford', NULL, NULL, 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.18, -72.93, 1140),
(1009, 'PO BOX', b'0', 'Bondsville', NULL, NULL, 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.20, -72.34, 1238),
(1010, 'STANDARD', b'0', 'Brimfield', NULL, 'East Brimfield', 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.11, -72.20, 3420),
(1011, 'STANDARD', b'0', 'Chester', NULL, NULL, 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.28, -72.98, 1090),
(1012, 'STANDARD', b'0', 'Chesterfield', NULL, NULL, 'MA', 'Hampshire County', 'America/New_York', '413', 'NA', 'US', 42.40, -72.85, 610),
(1013, 'STANDARD', b'0', 'Chicopee', 'Willimansett', NULL, 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.15, -72.60, 18880),
(1014, 'PO BOX', b'0', 'Chicopee', NULL, NULL, 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.17, -72.57, 354),
(1020, 'STANDARD', b'0', 'Chicopee', NULL, NULL, 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.17, -72.57, 25790),
(1021, 'PO BOX', b'0', 'Chicopee', NULL, NULL, 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.17, -72.57, 566),
(1022, 'STANDARD', b'0', 'Chicopee', 'Westover Afb', NULL, 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.20, -72.54, 1780),
(1026, 'STANDARD', b'0', 'Cummington', NULL, 'West Cummington', 'MA', 'Hampshire County', 'America/New_York', '413', 'NA', 'US', 42.46, -72.90, 890),
(1027, 'STANDARD', b'0', 'Easthampton', 'E Hampton, Mount Tom, Westhampton', 'Loudville', 'MA', 'Hampshire County', 'America/New_York', '413', 'NA', 'US', 42.26, -72.68, 16320),
(1028, 'STANDARD', b'0', 'East Longmeadow', 'E Longmeadow', NULL, 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.06, -72.51, 15330),
(1029, 'PO BOX', b'0', 'East Otis', NULL, 'Big Pond, E Otis', 'MA', 'Berkshire County', 'America/New_York', '413', 'NA', 'US', 42.17, -73.03, 557),
(1030, 'STANDARD', b'0', 'Feeding Hills', NULL, NULL, 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.06, -72.67, 11010),
(1031, 'STANDARD', b'0', 'Gilbertville', NULL, 'Old Furnace', 'MA', 'Worcester County', 'America/New_York', '413', 'NA', 'US', 42.33, -72.20, 1000),
(1032, 'STANDARD', b'0', 'Goshen', NULL, 'Lithia', 'MA', 'Hampshire County', 'America/New_York', '413', 'NA', 'US', 42.46, -72.81, 470),
(1033, 'STANDARD', b'0', 'Granby', NULL, NULL, 'MA', 'Hampshire County', 'America/New_York', '413', 'NA', 'US', 42.26, -72.52, 5890),
(1034, 'STANDARD', b'0', 'Granville', 'Tolland', 'Granville Center, West Granville', 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.06, -72.86, 1860),
(1035, 'STANDARD', b'0', 'Hadley', NULL, 'North Hadley', 'MA', 'Hampshire County', 'America/New_York', '413', 'NA', 'US', 42.35, -72.58, 4620),
(1036, 'STANDARD', b'0', 'Hampden', NULL, 'Hampton', 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.06, -72.41, 4880),
(1037, 'PO BOX', b'0', 'Hardwick', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', NULL, 'NA', 'US', 42.35, -72.20, 779),
(1038, 'STANDARD', b'0', 'Hatfield', NULL, 'West Hatfield', 'MA', 'Hampshire County', 'America/New_York', '413', 'NA', 'US', 42.37, -72.60, 2280),
(1039, 'STANDARD', b'0', 'Haydenville', 'West Whately', NULL, 'MA', 'Hampshire County', 'America/New_York', NULL, 'NA', 'US', 42.39, -72.70, 1240),
(1040, 'STANDARD', b'0', 'Holyoke', NULL, 'Halyoke', 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.21, -72.64, 29750),
(1041, 'PO BOX', b'0', 'Holyoke', NULL, 'Halyoke', 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.21, -72.64, 1408),
(1050, 'STANDARD', b'0', 'Huntington', 'Montgomery', 'Crescent Mills, Hntgtn, Knightville, North Chester, South Worthington', 'MA', 'Hampshire County', 'America/New_York', '413', 'NA', 'US', 42.23, -72.88, 2270),
(1053, 'STANDARD', b'0', 'Leeds', NULL, NULL, 'MA', 'Hampshire County', 'America/New_York', '413', 'NA', 'US', 42.35, -72.71, 1520),
(1054, 'STANDARD', b'0', 'Leverett', NULL, 'East Leverett, North Leverett', 'MA', 'Franklin County', 'America/New_York', NULL, 'NA', 'US', 42.45, -72.50, 1750),
(1056, 'STANDARD', b'0', 'Ludlow', NULL, NULL, 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.16, -72.48, 18130),
(1057, 'STANDARD', b'0', 'Monson', NULL, NULL, 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.09, -72.31, 7600),
(1059, 'PO BOX', b'0', 'North Amherst', 'Amherst', NULL, 'MA', 'Hampshire County', 'America/New_York', '413', 'NA', 'US', 42.40, -72.52, 70),
(1060, 'STANDARD', b'0', 'Northampton', NULL, 'North Hampton', 'MA', 'Hampshire County', 'America/New_York', '413', 'NA', 'US', 42.32, -72.63, 11260),
(1061, 'PO BOX', b'0', 'Northampton', NULL, 'North Hampton', 'MA', 'Hampshire County', 'America/New_York', '413', 'NA', 'US', 42.32, -72.67, 469),
(1062, 'STANDARD', b'0', 'Florence', 'Bay State Village, Bay State Vlg, Northampton', 'North Hampton', 'MA', 'Hampshire County', 'America/New_York', '413', 'NA', 'US', 42.32, -72.67, 10090),
(1063, 'UNIQUE', b'0', 'Northampton', NULL, 'North Hampton, Smith College', 'MA', 'Hampshire County', 'America/New_York', '413', 'NA', 'US', 42.32, -72.64, 177),
(1066, 'PO BOX', b'0', 'North Hatfield', 'N Hatfield', 'No Hatfield', 'MA', 'Hampshire County', 'America/New_York', '413', 'NA', 'US', 42.41, -72.66, 402),
(1068, 'STANDARD', b'0', 'Oakham', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '508,774', 'NA', 'US', 42.35, -72.05, 1790),
(1069, 'STANDARD', b'0', 'Palmer', NULL, NULL, 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.16, -72.32, 7190),
(1070, 'STANDARD', b'0', 'Plainfield', NULL, NULL, 'MA', 'Hampshire County', 'America/New_York', '413', 'NA', 'US', 42.51, -72.91, 540),
(1071, 'STANDARD', b'0', 'Russell', NULL, NULL, 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.18, -72.85, 1390),
(1072, 'STANDARD', b'0', 'Shutesbury', NULL, NULL, 'MA', 'Franklin County', 'America/New_York', NULL, 'NA', 'US', 42.45, -72.40, 1350),
(1073, 'STANDARD', b'0', 'Southampton', NULL, NULL, 'MA', 'Hampshire County', 'America/New_York', '413', 'NA', 'US', 42.23, -72.73, 5980),
(1074, 'PO BOX', b'0', 'South Barre', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '351', 'NA', 'US', 42.39, -72.09, 655),
(1075, 'STANDARD', b'0', 'South Hadley', NULL, 'S Hadley, So Hadley, South Hadley Falls', 'MA', 'Hampshire County', 'America/New_York', '413', 'NA', 'US', 42.26, -72.56, 14590),
(1077, 'STANDARD', b'0', 'Southwick', NULL, NULL, 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.05, -72.76, 8820),
(1079, 'PO BOX', b'0', 'Thorndike', NULL, NULL, 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.20, -72.33, 992),
(1080, 'STANDARD', b'0', 'Three Rivers', NULL, NULL, 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.18, -72.37, 2000),
(1081, 'STANDARD', b'0', 'Wales', NULL, NULL, 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.06, -72.21, 1490),
(1082, 'STANDARD', b'0', 'Ware', 'Hardwick', NULL, 'MA', 'Hampshire County', 'America/New_York', '413', 'NA', 'US', 42.25, -72.24, 8840),
(1083, 'PO BOX', b'0', 'Warren', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', NULL, 'NA', 'US', 42.21, -72.19, 3037),
(1084, 'STANDARD', b'0', 'West Chesterfield', 'W Chesterfld', NULL, 'MA', 'Hampshire County', 'America/New_York', '413', 'NA', 'US', 42.39, -72.88, 137),
(1085, 'STANDARD', b'0', 'Westfield', 'Montgomery', NULL, 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.13, -72.75, 34780),
(1086, 'PO BOX', b'0', 'Westfield', NULL, NULL, 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.13, -72.79, 886),
(1088, 'STANDARD', b'0', 'West Hatfield', 'W Hatfield', NULL, 'MA', 'Hampshire County', 'America/New_York', '413', 'NA', 'US', 42.39, -72.64, 490),
(1089, 'STANDARD', b'0', 'West Springfield', 'W Springfield', 'West Springfld', 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.12, -72.65, 25520),
(1090, 'PO BOX', b'0', 'West Springfield', 'W Springfield', NULL, 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.12, -72.65, 510),
(1092, 'PO BOX', b'0', 'West Warren', NULL, 'W Warren', 'MA', 'Worcester County', 'America/New_York', NULL, 'NA', 'US', 42.19, -72.24, 1332),
(1093, 'PO BOX', b'0', 'Whately', NULL, NULL, 'MA', 'Franklin County', 'America/New_York', NULL, 'NA', 'US', 42.44, -72.65, 516),
(1094, 'PO BOX', b'0', 'Wheelwright', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', NULL, 'NA', 'US', 42.35, -72.14, 340),
(1095, 'STANDARD', b'0', 'Wilbraham', NULL, NULL, 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.13, -72.43, 13840),
(1096, 'STANDARD', b'0', 'Williamsburg', NULL, 'S Chesterfield, South Chesterfield', 'MA', 'Hampshire County', 'America/New_York', '413', 'NA', 'US', 42.40, -72.76, 2310),
(1097, 'PO BOX', b'0', 'Woronoco', NULL, NULL, 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.18, -72.83, 88),
(1098, 'STANDARD', b'0', 'Worthington', NULL, NULL, 'MA', 'Hampshire County', 'America/New_York', '413', 'NA', 'US', 42.41, -72.93, 1060),
(1101, 'PO BOX', b'0', 'Springfield', NULL, 'Spfld', 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.11, -72.53, 2038),
(1102, 'PO BOX', b'0', 'Springfield', NULL, 'Spfld', 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.11, -72.53, 0),
(1103, 'STANDARD', b'0', 'Springfield', NULL, 'Spfld', 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.10, -72.59, 1580),
(1104, 'STANDARD', b'0', 'Springfield', NULL, 'Spfld', 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.13, -72.57, 19290),
(1105, 'STANDARD', b'0', 'Springfield', NULL, 'Spfld', 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.10, -72.58, 8040),
(1106, 'STANDARD', b'0', 'Longmeadow', 'Spfld, Springfield', NULL, 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.04, -72.57, 15400),
(1107, 'STANDARD', b'0', 'Springfield', NULL, 'Brightwood, Spfld', 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.12, -72.61, 8420),
(1108, 'STANDARD', b'0', 'Springfield', NULL, 'Spfld', 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.08, -72.56, 21860),
(1109, 'STANDARD', b'0', 'Springfield', NULL, 'Spfld', 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.11, -72.53, 21920),
(1111, 'UNIQUE', b'0', 'Springfield', NULL, 'Mass Mutual Life Ins Co', 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.11, -72.53, 0),
(1115, 'PO BOX', b'0', 'Springfield', NULL, 'Bay State W Tower', 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.11, -72.53, 25),
(1116, 'PO BOX', b'0', 'Longmeadow', 'E Longmeadow, East Longmeadow', NULL, 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.04, -72.57, 82),
(1118, 'STANDARD', b'0', 'Springfield', NULL, 'Spfld', 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.09, -72.53, 12820),
(1119, 'STANDARD', b'0', 'Springfield', NULL, 'Spfld', 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.12, -72.51, 11040),
(1128, 'STANDARD', b'0', 'Springfield', NULL, 'Spfld', 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.09, -72.49, 2610),
(1129, 'STANDARD', b'0', 'Springfield', NULL, 'Spfld', 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.12, -72.49, 6540),
(1133, 'UNIQUE', b'1', 'Springfield', NULL, 'Monarch Life Ins Co', 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.10, -72.59, 0),
(1138, 'PO BOX', b'0', 'Springfield', NULL, 'Spfld', 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.11, -72.53, 688),
(1139, 'PO BOX', b'0', 'Springfield', NULL, 'Spfld', 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.11, -72.53, 467),
(1144, 'STANDARD', b'0', 'Springfield', NULL, NULL, 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.11, -72.53, 0),
(1151, 'STANDARD', b'0', 'Indian Orchard', 'Indian Orch, Springfield', 'Spfld', 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.15, -72.51, 7630),
(1152, 'STANDARD', b'0', 'Springfield', NULL, 'General Mail Facility-bmc', 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.11, -72.53, 0),
(1195, 'STANDARD', b'1', 'Springfield', 'Springfield Bmc', NULL, 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.10, -72.58, 0),
(1199, 'UNIQUE', b'0', 'Springfield', NULL, 'Baystate Medical, Spfld', 'MA', 'Hampden County', 'America/New_York', '413', 'NA', 'US', 42.12, -72.60, 0),
(1201, 'STANDARD', b'0', 'Pittsfield', NULL, 'Allendale', 'MA', 'Berkshire County', 'America/New_York', '413', 'NA', 'US', 42.45, -73.26, 38200),
(1202, 'PO BOX', b'0', 'Pittsfield', NULL, NULL, 'MA', 'Berkshire County', 'America/New_York', '413', 'NA', 'US', 42.45, -73.26, 1387),
(1203, 'PO BOX', b'0', 'Pittsfield', NULL, NULL, 'MA', 'Berkshire County', 'America/New_York', '413', 'NA', 'US', 42.45, -73.26, 0),
(1220, 'STANDARD', b'0', 'Adams', NULL, NULL, 'MA', 'Berkshire County', 'America/New_York', '413', 'NA', 'US', 42.62, -73.11, 7260),
(1222, 'STANDARD', b'0', 'Ashley Falls', NULL, NULL, 'MA', 'Berkshire County', 'America/New_York', '413', 'NA', 'US', 42.05, -73.33, 730),
(1223, 'STANDARD', b'0', 'Becket', 'Washington', 'Becket Corners, Sherwood Forest', 'MA', 'Berkshire County', 'America/New_York', '413', 'NA', 'US', 42.33, -73.07, 1930),
(1224, 'STANDARD', b'0', 'Berkshire', 'Lanesboro', NULL, 'MA', 'Berkshire County', 'America/New_York', '413', 'NA', 'US', 42.51, -73.20, 160),
(1225, 'STANDARD', b'0', 'Cheshire', NULL, NULL, 'MA', 'Berkshire County', 'America/New_York', '413', 'NA', 'US', 42.55, -73.15, 2970),
(1226, 'STANDARD', b'0', 'Dalton', NULL, NULL, 'MA', 'Berkshire County', 'America/New_York', '413', 'NA', 'US', 42.47, -73.16, 5910),
(1227, 'PO BOX', b'0', 'Dalton', NULL, NULL, 'MA', 'Berkshire County', 'America/New_York', '413', 'NA', 'US', 42.47, -73.16, 262),
(1229, 'PO BOX', b'0', 'Glendale', NULL, NULL, 'MA', 'Berkshire County', 'America/New_York', '413', 'NA', 'US', 42.28, -73.34, 163),
(1230, 'STANDARD', b'0', 'Great Barrington', 'Egremont, Gt Barrington, N Egremont, New Marlboro, New Marlborou, New Marlborough, North Egremont, Simons Rock', 'Alford, Berkshire Heights, Hartsville, Risingdale, Van Deusenville', 'MA', 'Berkshire County', 'America/New_York', '413', 'NA', 'US', 42.19, -73.35, 6260),
(1235, 'STANDARD', b'0', 'Hinsdale', 'Peru', NULL, 'MA', 'Berkshire County', 'America/New_York', '413', 'NA', 'US', 42.42, -73.07, 2580),
(1236, 'STANDARD', b'0', 'Housatonic', NULL, NULL, 'MA', 'Berkshire County', 'America/New_York', '413', 'NA', 'US', 42.27, -73.38, 1420),
(1237, 'STANDARD', b'0', 'Lanesboro', 'Hancock, New Ashford', 'Lanesborough', 'MA', 'Berkshire County', 'America/New_York', '413', 'NA', 'US', 42.51, -73.22, 2720),
(1238, 'STANDARD', b'0', 'Lee', NULL, 'W Becket', 'MA', 'Berkshire County', 'America/New_York', '413', 'NA', 'US', 42.30, -73.25, 5160),
(1240, 'STANDARD', b'0', 'Lenox', NULL, NULL, 'MA', 'Berkshire County', 'America/New_York', '413', 'NA', 'US', 42.35, -73.28, 4110),
(1242, 'PO BOX', b'0', 'Lenox Dale', NULL, NULL, 'MA', 'Berkshire County', 'America/New_York', '413', 'NA', 'US', 42.33, -73.25, 559),
(1243, 'PO BOX', b'0', 'Middlefield', NULL, NULL, 'MA', 'Hampshire County', 'America/New_York', '413', 'NA', 'US', 42.35, -73.01, 315),
(1244, 'PO BOX', b'0', 'Mill River', NULL, NULL, 'MA', 'Berkshire County', 'America/New_York', '413', 'NA', 'US', 42.12, -73.26, 328),
(1245, 'STANDARD', b'0', 'Monterey', 'West Otis', NULL, 'MA', 'Berkshire County', 'America/New_York', '413', 'NA', 'US', 42.18, -73.21, 680),
(1247, 'STANDARD', b'0', 'North Adams', 'Clarksburg, Florida', 'N Adams, No Adams', 'MA', 'Berkshire County', 'America/New_York', '413', 'NA', 'US', 42.68, -73.11, 11710),
(1252, 'PO BOX', b'0', 'North Egremont', 'N Egremont', 'No Egremont', 'MA', 'Berkshire County', 'America/New_York', '413', 'NA', 'US', 42.17, -73.44, 281),
(1253, 'STANDARD', b'0', 'Otis', NULL, 'Cold Spring, North Otis', 'MA', 'Berkshire County', 'America/New_York', '413', 'NA', 'US', 42.21, -73.11, 740),
(1254, 'STANDARD', b'0', 'Richmond', NULL, NULL, 'MA', 'Berkshire County', 'America/New_York', '413', 'NA', 'US', 42.37, -73.36, 1060),
(1255, 'STANDARD', b'0', 'Sandisfield', NULL, 'South Sandisfield', 'MA', 'Berkshire County', 'America/New_York', '413', 'NA', 'US', 42.11, -73.13, 600),
(1256, 'STANDARD', b'0', 'Savoy', NULL, NULL, 'MA', 'Berkshire County', 'America/New_York', '413', 'NA', 'US', 42.56, -73.02, 590),
(1257, 'STANDARD', b'0', 'Sheffield', NULL, NULL, 'MA', 'Berkshire County', 'America/New_York', '413', 'NA', 'US', 42.10, -73.34, 2080),
(1258, 'STANDARD', b'0', 'South Egremont', 'Mount Washington, Mt Washington, S Egremont', 'So Egremont', 'MA', 'Berkshire County', 'America/New_York', '413', 'NA', 'US', 42.11, -73.46, 600),
(1259, 'STANDARD', b'0', 'Southfield', NULL, NULL, 'MA', 'Berkshire County', 'America/New_York', '413', 'NA', 'US', 42.07, -73.23, 410),
(1260, 'PO BOX', b'0', 'South Lee', NULL, NULL, 'MA', 'Berkshire County', 'America/New_York', '413', 'NA', 'US', 42.30, -73.34, 254),
(1262, 'PO BOX', b'0', 'Stockbridge', NULL, NULL, 'MA', 'Berkshire County', 'America/New_York', '413', 'NA', 'US', 42.30, -73.32, 1342),
(1263, 'UNIQUE', b'0', 'Stockbridge', NULL, 'Assoc Of Marian Helpers, Marian Helpers, Marion Fathers', 'MA', 'Berkshire County', 'America/New_York', '413', 'NA', 'US', 42.30, -73.32, 0),
(1264, 'PO BOX', b'0', 'Tyringham', 'Lee', NULL, 'MA', 'Berkshire County', 'America/New_York', '413', 'NA', 'US', 42.24, -73.19, 198),
(1266, 'STANDARD', b'0', 'West Stockbridge', 'Alford, W Stockbridge', 'Interlaken, West Stockbridge Center', 'MA', 'Berkshire County', 'America/New_York', '413', 'NA', 'US', 42.31, -73.39, 1240),
(1267, 'STANDARD', b'0', 'Williamstown', NULL, 'Williamstn, Wmstown', 'MA', 'Berkshire County', 'America/New_York', '413', 'NA', 'US', 42.70, -73.20, 5170),
(1270, 'STANDARD', b'0', 'Windsor', NULL, 'East Windsor', 'MA', 'Berkshire County', 'America/New_York', '413', 'NA', 'US', 42.51, -73.04, 710),
(1301, 'STANDARD', b'0', 'Greenfield', 'Leyden', NULL, 'MA', 'Franklin County', 'America/New_York', '413', 'NA', 'US', 42.58, -72.59, 14160),
(1302, 'PO BOX', b'0', 'Greenfield', NULL, NULL, 'MA', 'Franklin County', 'America/New_York', '413', 'NA', 'US', 42.58, -72.59, 502),
(1330, 'STANDARD', b'0', 'Ashfield', NULL, 'South Ashfield', 'MA', 'Franklin County', 'America/New_York', '413', 'NA', 'US', 42.53, -72.78, 1310),
(1331, 'STANDARD', b'0', 'Athol', 'Phillipston', 'Royalston', 'MA', 'Worcester County', 'America/New_York', '978', 'NA', 'US', 42.59, -72.23, 11210),
(1337, 'STANDARD', b'0', 'Bernardston', 'Leyden', NULL, 'MA', 'Franklin County', 'America/New_York', '413', 'NA', 'US', 42.66, -72.55, 2440),
(1338, 'STANDARD', b'0', 'Buckland', NULL, NULL, 'MA', 'Franklin County', 'America/New_York', '413', 'NA', 'US', 42.57, -72.82, 210),
(1339, 'STANDARD', b'0', 'Charlemont', 'Hawley', 'West Hawley', 'MA', 'Franklin County', 'America/New_York', NULL, 'NA', 'US', 42.63, -72.88, 1240),
(1340, 'STANDARD', b'0', 'Colrain', 'Shattuckville', NULL, 'MA', 'Franklin County', 'America/New_York', '413', 'NA', 'US', 42.66, -72.68, 1600),
(1341, 'STANDARD', b'0', 'Conway', NULL, NULL, 'MA', 'Franklin County', 'America/New_York', '413', 'NA', 'US', 42.51, -72.68, 1560),
(1342, 'STANDARD', b'0', 'Deerfield', NULL, 'East Deerfield, West Deerfield', 'MA', 'Franklin County', 'America/New_York', '413', 'NA', 'US', 42.55, -72.60, 1340),
(1343, 'STANDARD', b'0', 'Drury', NULL, 'Charlemont', 'MA', 'Berkshire County', 'America/New_York', '413', 'NA', 'US', 42.66, -72.98, 153),
(1344, 'STANDARD', b'0', 'Erving', NULL, 'Farley, Stoneville', 'MA', 'Franklin County', 'America/New_York', NULL, 'NA', 'US', 42.60, -72.40, 1390),
(1346, 'STANDARD', b'0', 'Heath', 'Charlemont', NULL, 'MA', 'Franklin County', 'America/New_York', '413', 'NA', 'US', 42.66, -72.83, 340),
(1347, 'PO BOX', b'0', 'Lake Pleasant', NULL, NULL, 'MA', 'Franklin County', 'America/New_York', NULL, 'NA', 'US', 42.56, -72.52, 165),
(1349, 'STANDARD', b'0', 'Millers Falls', NULL, 'Turners Falls', 'MA', 'Franklin County', 'America/New_York', '413', 'NA', 'US', 42.56, -72.48, 720),
(1350, 'PO BOX', b'0', 'Monroe Bridge', 'Monroe', NULL, 'MA', 'Franklin County', 'America/New_York', NULL, 'NA', 'US', 42.72, -72.98, 35),
(1351, 'STANDARD', b'0', 'Montague', NULL, NULL, 'MA', 'Franklin County', 'America/New_York', '413', 'NA', 'US', 42.53, -72.53, 2110),
(1354, 'STANDARD', b'0', 'Gill', 'Mount Hermon, Mt Hermon, Northfield Mount Hermon, Northfield Mt Hermon', NULL, 'MA', 'Franklin County', 'America/New_York', '413', 'NA', 'US', 42.62, -72.51, 1450),
(1355, 'STANDARD', b'0', 'New Salem', NULL, 'Orange', 'MA', 'Franklin County', 'America/New_York', '978', 'NA', 'US', 42.50, -72.33, 860),
(1360, 'STANDARD', b'0', 'Northfield', NULL, 'N Field, No Field', 'MA', 'Franklin County', 'America/New_York', '413', 'NA', 'US', 42.70, -72.43, 2680),
(1364, 'STANDARD', b'0', 'Orange', 'Warwick', 'Blissville, Eagleville, Lake Mattawa, N Orange, New Salem, North Orange', 'MA', 'Franklin County', 'America/New_York', '978', 'NA', 'US', 42.59, -72.30, 6140),
(1366, 'STANDARD', b'0', 'Petersham', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '978', 'NA', 'US', 42.48, -72.18, 1090),
(1367, 'STANDARD', b'0', 'Rowe', NULL, 'Hoosac Tunnel, Zoar', 'MA', 'Franklin County', 'America/New_York', '413', 'NA', 'US', 42.70, -72.90, 440),
(1368, 'STANDARD', b'0', 'Royalston', 'S Royalston', 'Athol', 'MA', 'Worcester County', 'America/New_York', '978', 'NA', 'US', 42.68, -72.18, 1050),
(1370, 'STANDARD', b'0', 'Shelburne Falls', 'Shelburne Fls', 'Baptist Corner, East Charlemont, Shelburne', 'MA', 'Franklin County', 'America/New_York', '413', 'NA', 'US', 42.60, -72.74, 3510),
(1373, 'STANDARD', b'0', 'South Deerfield', 'S Deerfield', 'So Deerfield, Whately', 'MA', 'Franklin County', 'America/New_York', '413', 'NA', 'US', 42.47, -72.59, 4070),
(1375, 'STANDARD', b'0', 'Sunderland', NULL, NULL, 'MA', 'Franklin County', 'America/New_York', '413', 'NA', 'US', 42.46, -72.58, 2880),
(1376, 'STANDARD', b'0', 'Turners Falls', NULL, NULL, 'MA', 'Franklin County', 'America/New_York', '413', 'NA', 'US', 42.59, -72.55, 4320),
(1378, 'STANDARD', b'0', 'Warwick', 'Orange', NULL, 'MA', 'Franklin County', 'America/New_York', '978', 'NA', 'US', 42.68, -72.33, 630),
(1379, 'STANDARD', b'0', 'Wendell', NULL, NULL, 'MA', 'Franklin County', 'America/New_York', NULL, 'NA', 'US', 42.55, -72.40, 750),
(1380, 'STANDARD', b'0', 'Wendell Depot', NULL, NULL, 'MA', 'Franklin County', 'America/New_York', NULL, 'NA', 'US', 42.58, -72.37, 81),
(1420, 'STANDARD', b'0', 'Fitchburg', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '978,508', 'NA', 'US', 42.58, -71.81, 33060),
(1430, 'STANDARD', b'0', 'Ashburnham', NULL, 'South Ashburnham', 'MA', 'Worcester County', 'America/New_York', '978', 'NA', 'US', 42.63, -71.90, 5730),
(1431, 'STANDARD', b'0', 'Ashby', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', '978', 'NA', 'US', 42.68, -71.81, 2860),
(1432, 'STANDARD', b'0', 'Ayer', NULL, 'Devens, Fort Devens, Ft Devens', 'MA', 'Middlesex County', 'America/New_York', '978', 'NA', 'US', 42.56, -71.58, 7280),
(1434, 'STANDARD', b'0', 'Devens', 'Ayer', NULL, 'MA', 'Worcester County', 'America/New_York', NULL, 'NA', 'US', 42.53, -71.61, 440),
(1436, 'STANDARD', b'0', 'Baldwinville', NULL, 'Otter River', 'MA', 'Worcester County', 'America/New_York', NULL, 'NA', 'US', 42.60, -72.07, 2580),
(1438, 'PO BOX', b'0', 'East Templeton', 'E Templeton', NULL, 'MA', 'Worcester County', 'America/New_York', NULL, 'NA', 'US', 42.56, -72.03, 704),
(1440, 'STANDARD', b'0', 'Gardner', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '978', 'NA', 'US', 42.58, -71.98, 16400),
(1441, 'UNIQUE', b'0', 'Westminster', NULL, 'Gardner, Tyco', 'MA', 'Worcester County', 'America/New_York', '351', 'NA', 'US', 42.58, -71.98, 0),
(1450, 'STANDARD', b'0', 'Groton', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', '978', 'NA', 'US', 42.60, -71.57, 10720),
(1451, 'STANDARD', b'0', 'Harvard', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '978', 'NA', 'US', 42.50, -71.58, 5190),
(1452, 'STANDARD', b'0', 'Hubbardston', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '978', 'NA', 'US', 42.48, -72.01, 4150),
(1453, 'STANDARD', b'0', 'Leominster', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '508,978', 'NA', 'US', 42.51, -71.77, 37880),
(1460, 'STANDARD', b'0', 'Littleton', NULL, 'Pingryville', 'MA', 'Middlesex County', 'America/New_York', '508,781,978', 'NA', 'US', 42.53, -71.47, 9360),
(1462, 'STANDARD', b'0', 'Lunenburg', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '508,978', 'NA', 'US', 42.59, -71.72, 10460),
(1463, 'STANDARD', b'0', 'Pepperell', NULL, 'E Pepperell, East Pepperell', 'MA', 'Middlesex County', 'America/New_York', '351,978', 'NA', 'US', 42.66, -71.58, 10880),
(1464, 'STANDARD', b'0', 'Shirley', 'Shirley Center, Shirley Ctr', NULL, 'MA', 'Middlesex County', 'America/New_York', '978', 'NA', 'US', 42.54, -71.65, 5680),
(1467, 'PO BOX', b'0', 'Still River', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', NULL, 'NA', 'US', 42.49, -71.61, 318),
(1468, 'STANDARD', b'0', 'Templeton', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '978', 'NA', 'US', 42.55, -72.06, 3940),
(1469, 'STANDARD', b'0', 'Townsend', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', '978', 'NA', 'US', 42.66, -71.70, 6770),
(1470, 'UNIQUE', b'0', 'Groton', NULL, 'New England Business', 'MA', 'Middlesex County', 'America/New_York', '351', 'NA', 'US', 42.60, -71.57, 0),
(1471, 'UNIQUE', b'0', 'Groton', NULL, 'New England Business Svc Inc', 'MA', 'Middlesex County', 'America/New_York', '351', 'NA', 'US', 42.60, -71.57, 0),
(1472, 'PO BOX', b'0', 'West Groton', NULL, 'W Groton', 'MA', 'Middlesex County', 'America/New_York', NULL, 'NA', 'US', 42.61, -71.62, 352),
(1473, 'STANDARD', b'0', 'Westminster', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '978', 'NA', 'US', 42.55, -71.90, 7330),
(1474, 'STANDARD', b'0', 'West Townsend', 'Townsend, W Townsend', NULL, 'MA', 'Middlesex County', 'America/New_York', '978', 'NA', 'US', 42.66, -71.74, 1830),
(1475, 'STANDARD', b'0', 'Winchendon', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '978', 'NA', 'US', 42.68, -72.04, 8680),
(1477, 'PO BOX', b'0', 'Winchendon Springs', 'Winchdon Spgs', NULL, 'MA', 'Worcester County', 'America/New_York', '351', 'NA', 'US', 42.69, -72.01, 214),
(1501, 'STANDARD', b'0', 'Auburn', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '508,774', 'NA', 'US', 42.20, -71.83, 15470),
(1503, 'STANDARD', b'0', 'Berlin', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '978', 'NA', 'US', 42.38, -71.63, 2840),
(1504, 'STANDARD', b'0', 'Blackstone', NULL, 'E Blackstone, East Blackstone, Millerville', 'MA', 'Worcester County', 'America/New_York', '508,774', 'NA', 'US', 42.04, -71.53, 8380),
(1505, 'STANDARD', b'0', 'Boylston', NULL, 'Morningdale', 'MA', 'Worcester County', 'America/New_York', '508,774', 'NA', 'US', 42.35, -71.73, 4310),
(1506, 'STANDARD', b'0', 'Brookfield', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '413', 'NA', 'US', 42.21, -72.10, 3050),
(1507, 'STANDARD', b'0', 'Charlton', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '508,774', 'NA', 'US', 42.13, -71.96, 11860),
(1508, 'PO BOX', b'0', 'Charlton City', NULL, 'Richardson Corners', 'MA', 'Worcester County', 'America/New_York', NULL, 'NA', 'US', 42.13, -71.96, 958),
(1509, 'PO BOX', b'0', 'Charlton Depot', 'Charlton Dept, Charlton Dpt', NULL, 'MA', 'Worcester County', 'America/New_York', NULL, 'NA', 'US', 42.13, -71.96, 52),
(1510, 'STANDARD', b'0', 'Clinton', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '978', 'NA', 'US', 42.41, -71.68, 13050),
(1515, 'STANDARD', b'0', 'East Brookfield', 'E Brookfield', NULL, 'MA', 'Worcester County', 'America/New_York', '508,774', 'NA', 'US', 42.22, -72.04, 2140),
(1516, 'STANDARD', b'0', 'Douglas', 'East Douglas', NULL, 'MA', 'Worcester County', 'America/New_York', '508,774', 'NA', 'US', 42.05, -71.73, 8430),
(1517, 'PO BOX', b'0', 'East Princeton', 'E Princeton', NULL, 'MA', 'Worcester County', 'America/New_York', NULL, 'NA', 'US', 42.46, -71.89, 0),
(1518, 'STANDARD', b'0', 'Fiskdale', 'Sturbridge', NULL, 'MA', 'Worcester County', 'America/New_York', '774,508', 'NA', 'US', 42.12, -72.11, 2990),
(1519, 'STANDARD', b'0', 'Grafton', NULL, 'Hassanamisco Indian Reservat', 'MA', 'Worcester County', 'America/New_York', '508,774', 'NA', 'US', 42.20, -71.68, 6990);
INSERT INTO `zips` (`zip`, `type`, `decommissioned`, `primary_city`, `acceptable_cities`, `unacceptable_cities`, `state`, `county`, `timezone`, `area_codes`, `world_region`, `country`, `latitude`, `longitude`, `irs_estimated_population_2015`) VALUES
(1520, 'STANDARD', b'0', 'Holden', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '508,774', 'NA', 'US', 42.35, -71.85, 15210),
(1521, 'STANDARD', b'0', 'Holland', 'Fiskdale', 'Halland', 'MA', 'Hampden County', 'America/New_York', '508', 'NA', 'US', 42.05, -72.15, 2180),
(1522, 'STANDARD', b'0', 'Jefferson', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', NULL, 'NA', 'US', 42.38, -71.87, 3240),
(1523, 'STANDARD', b'0', 'Lancaster', NULL, 'North Lancaster', 'MA', 'Worcester County', 'America/New_York', NULL, 'NA', 'US', 42.45, -71.66, 5860),
(1524, 'STANDARD', b'0', 'Leicester', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '508,774', 'NA', 'US', 42.25, -71.90, 6030),
(1525, 'PO BOX', b'0', 'Linwood', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', NULL, 'NA', 'US', 42.11, -71.63, 1058),
(1526, 'PO BOX', b'0', 'Manchaug', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', NULL, 'NA', 'US', 42.12, -71.76, 534),
(1527, 'STANDARD', b'0', 'Millbury', NULL, 'East Millbury', 'MA', 'Worcester County', 'America/New_York', '508,774', 'NA', 'US', 42.19, -71.76, 12130),
(1529, 'STANDARD', b'0', 'Millville', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', NULL, 'NA', 'US', 42.03, -71.58, 2950),
(1531, 'STANDARD', b'0', 'New Braintree', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '508,774', 'NA', 'US', 42.31, -72.13, 950),
(1532, 'STANDARD', b'0', 'Northborough', NULL, 'Northboro', 'MA', 'Worcester County', 'America/New_York', '508,774', 'NA', 'US', 42.31, -71.64, 15300),
(1534, 'STANDARD', b'0', 'Northbridge', NULL, 'Rockdale', 'MA', 'Worcester County', 'America/New_York', NULL, 'NA', 'US', 42.15, -71.65, 5830),
(1535, 'STANDARD', b'0', 'North Brookfield', 'N Brookfield', NULL, 'MA', 'Worcester County', 'America/New_York', '508,774', 'NA', 'US', 42.27, -72.08, 4150),
(1536, 'STANDARD', b'0', 'North Grafton', NULL, 'N Grafton, No Grafton', 'MA', 'Worcester County', 'America/New_York', NULL, 'NA', 'US', 42.23, -71.69, 6880),
(1537, 'STANDARD', b'0', 'North Oxford', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '508', 'NA', 'US', 42.16, -71.88, 2000),
(1538, 'PO BOX', b'0', 'North Uxbridge', 'N Uxbridge', NULL, 'MA', 'Worcester County', 'America/New_York', NULL, 'NA', 'US', 42.05, -71.64, 584),
(1540, 'STANDARD', b'0', 'Oxford', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '508,774', 'NA', 'US', 42.11, -71.87, 10400),
(1541, 'STANDARD', b'0', 'Princeton', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '978', 'NA', 'US', 42.45, -71.86, 3480),
(1542, 'STANDARD', b'0', 'Rochdale', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', NULL, 'NA', 'US', 42.20, -71.90, 2100),
(1543, 'STANDARD', b'0', 'Rutland', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '508,774', 'NA', 'US', 42.36, -71.95, 8260),
(1545, 'STANDARD', b'0', 'Shrewsbury', NULL, 'Edgemere', 'MA', 'Worcester County', 'America/New_York', '508,774', 'NA', 'US', 42.30, -71.71, 36050),
(1546, 'UNIQUE', b'0', 'Shrewsbury', NULL, 'Central Mass P & D Ctr', 'MA', 'Worcester County', 'America/New_York', '508', 'NA', 'US', 42.30, -71.71, 0),
(1550, 'STANDARD', b'0', 'Southbridge', NULL, 'Globe Village, Sandersdale', 'MA', 'Worcester County', 'America/New_York', '508,774', 'NA', 'US', 42.08, -72.03, 14970),
(1560, 'STANDARD', b'0', 'South Grafton', NULL, 'Saundersville', 'MA', 'Worcester County', 'America/New_York', NULL, 'NA', 'US', 42.17, -71.68, 4330),
(1561, 'PO BOX', b'0', 'South Lancaster', 'S Lancaster', 'So Lancaster', 'MA', 'Worcester County', 'America/New_York', NULL, 'NA', 'US', 42.44, -71.69, 1100),
(1562, 'STANDARD', b'0', 'Spencer', NULL, 'Lambs Grove', 'MA', 'Worcester County', 'America/New_York', '508,774', 'NA', 'US', 42.24, -71.99, 10260),
(1564, 'STANDARD', b'0', 'Sterling', NULL, 'Sterling Junction', 'MA', 'Worcester County', 'America/New_York', '978', 'NA', 'US', 42.43, -71.75, 7630),
(1566, 'STANDARD', b'0', 'Sturbridge', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '508,774', 'NA', 'US', 42.09, -72.06, 6400),
(1568, 'STANDARD', b'0', 'Upton', NULL, 'W Upton, West Upton', 'MA', 'Worcester County', 'America/New_York', '508,774', 'NA', 'US', 42.17, -71.61, 7420),
(1569, 'STANDARD', b'0', 'Uxbridge', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '774,508', 'NA', 'US', 42.08, -71.60, 12610),
(1570, 'STANDARD', b'0', 'Webster', 'Dudley Hill', NULL, 'MA', 'Worcester County', 'America/New_York', '508,774', 'NA', 'US', 42.04, -71.87, 14980),
(1571, 'STANDARD', b'0', 'Dudley', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', NULL, 'NA', 'US', 42.05, -71.93, 10050),
(1580, 'UNIQUE', b'0', 'Westborough', NULL, 'Emc, Westboro', 'MA', 'Worcester County', 'America/New_York', '508', 'NA', 'US', 42.26, -71.61, 0),
(1581, 'STANDARD', b'0', 'Westborough', NULL, 'Westboro', 'MA', 'Worcester County', 'America/New_York', '508,774', 'NA', 'US', 42.26, -71.61, 19460),
(1582, 'UNIQUE', b'0', 'Westborough', NULL, 'National Grid Co, Westboro', 'MA', 'Worcester County', 'America/New_York', '508', 'NA', 'US', 42.26, -71.61, 0),
(1583, 'STANDARD', b'0', 'West Boylston', NULL, 'Oakdale, Westboylston', 'MA', 'Worcester County', 'America/New_York', NULL, 'NA', 'US', 42.36, -71.78, 6550),
(1585, 'STANDARD', b'0', 'West Brookfield', 'W Brookfield', 'Westbrookfield', 'MA', 'Worcester County', 'America/New_York', '508,413', 'NA', 'US', 42.23, -72.14, 3710),
(1586, 'PO BOX', b'0', 'West Millbury', 'Millbury', NULL, 'MA', 'Worcester County', 'America/New_York', NULL, 'NA', 'US', 42.19, -71.76, 0),
(1588, 'STANDARD', b'0', 'Whitinsville', 'Linwood', NULL, 'MA', 'Worcester County', 'America/New_York', '508,774', 'NA', 'US', 42.11, -71.67, 9200),
(1590, 'STANDARD', b'0', 'Sutton', 'Wilkinsonvile, Wilkinsonville', NULL, 'MA', 'Worcester County', 'America/New_York', NULL, 'NA', 'US', 42.15, -71.76, 8700),
(1601, 'PO BOX', b'0', 'Worcester', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '508', 'NA', 'US', 42.26, -71.80, 322),
(1602, 'STANDARD', b'0', 'Worcester', NULL, 'West Side', 'MA', 'Worcester County', 'America/New_York', NULL, 'NA', 'US', 42.27, -71.85, 19650),
(1603, 'STANDARD', b'0', 'Worcester', NULL, 'Webster Square', 'MA', 'Worcester County', 'America/New_York', '508', 'NA', 'US', 42.24, -71.84, 17400),
(1604, 'STANDARD', b'0', 'Worcester', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '508', 'NA', 'US', 42.25, -71.77, 31060),
(1605, 'STANDARD', b'0', 'Worcester', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '508', 'NA', 'US', 42.29, -71.79, 21860),
(1606, 'STANDARD', b'0', 'Worcester', NULL, 'Greendale', 'MA', 'Worcester County', 'America/New_York', '508', 'NA', 'US', 42.32, -71.80, 18160),
(1607, 'STANDARD', b'0', 'Worcester', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '508', 'NA', 'US', 42.23, -71.79, 7320),
(1608, 'STANDARD', b'0', 'Worcester', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '774,978,508', 'NA', 'US', 42.26, -71.80, 2790),
(1609, 'STANDARD', b'0', 'Worcester', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '508,774,978', 'NA', 'US', 42.29, -71.83, 12910),
(1610, 'STANDARD', b'0', 'Worcester', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '508,774', 'NA', 'US', 42.25, -71.81, 16530),
(1611, 'STANDARD', b'0', 'Cherry Valley', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', NULL, 'NA', 'US', 42.23, -71.87, 2100),
(1612, 'STANDARD', b'0', 'Paxton', 'Worcester', NULL, 'MA', 'Worcester County', 'America/New_York', NULL, 'NA', 'US', 42.31, -71.93, 4310),
(1613, 'PO BOX', b'0', 'Worcester', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '508', 'NA', 'US', 42.26, -71.80, 1369),
(1614, 'PO BOX', b'0', 'Worcester', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '508', 'NA', 'US', 42.26, -71.80, 68),
(1615, 'PO BOX', b'0', 'Worcester', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '508', 'NA', 'US', 42.26, -71.80, 13),
(1653, 'UNIQUE', b'0', 'Worcester', NULL, 'Allmerica', 'MA', 'Worcester County', 'America/New_York', '508', 'NA', 'US', 42.26, -71.80, 0),
(1654, 'UNIQUE', b'0', 'Worcester', NULL, 'Verizon', 'MA', 'Worcester County', 'America/New_York', '508', 'NA', 'US', 42.26, -71.80, 0),
(1655, 'STANDARD', b'0', 'Worcester', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '508,774,978', 'NA', 'US', 42.26, -71.80, 0),
(1701, 'STANDARD', b'0', 'Framingham', NULL, 'Framingham Center, Framingham So, Saxonville', 'MA', 'Middlesex County', 'America/New_York', '508,774,781,978', 'NA', 'US', 42.30, -71.43, 30670),
(1702, 'STANDARD', b'0', 'Framingham', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', '508,774,781,978', 'NA', 'US', 42.28, -71.44, 28760),
(1703, 'PO BOX', b'0', 'Framingham', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', '508', 'NA', 'US', 42.30, -71.43, 160),
(1704, 'PO BOX', b'0', 'Framingham', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', '508', 'NA', 'US', 42.30, -71.43, 385),
(1705, 'PO BOX', b'0', 'Framingham', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', '508', 'NA', 'US', 42.30, -71.43, 71),
(1718, 'STANDARD', b'0', 'Village Of Nagog Woods', 'Acton, Vlg Nagog Wds', 'Vlg Of Nagog Woods', 'MA', 'Middlesex County', 'America/New_York', '508,978,781', 'NA', 'US', 42.52, -71.43, 630),
(1719, 'STANDARD', b'0', 'Boxborough', 'Acton, Boxboro', NULL, 'MA', 'Middlesex County', 'America/New_York', '508,978', 'NA', 'US', 42.50, -71.50, 5020),
(1720, 'STANDARD', b'0', 'Acton', NULL, 'W Acton, West Acton', 'MA', 'Middlesex County', 'America/New_York', '781,508,978', 'NA', 'US', 42.48, -71.46, 22280),
(1721, 'STANDARD', b'0', 'Ashland', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', NULL, 'NA', 'US', 42.25, -71.46, 16840),
(1730, 'STANDARD', b'0', 'Bedford', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', '781,857', 'NA', 'US', 42.48, -71.26, 13570),
(1731, 'STANDARD', b'0', 'Hanscom Afb', 'Bedford', NULL, 'MA', 'Middlesex County', 'America/New_York', '857,781', 'NA', 'US', 42.46, -71.28, 2450),
(1740, 'STANDARD', b'0', 'Bolton', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '978', 'NA', 'US', 42.43, -71.60, 5230),
(1741, 'STANDARD', b'0', 'Carlisle', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', NULL, 'NA', 'US', 42.53, -71.35, 5220),
(1742, 'STANDARD', b'0', 'Concord', NULL, 'W Concord, West Concord', 'MA', 'Middlesex County', 'America/New_York', '978', 'NA', 'US', 42.45, -71.35, 17170),
(1745, 'STANDARD', b'0', 'Fayville', 'Southborough', 'Southboro', 'MA', 'Worcester County', 'America/New_York', NULL, 'NA', 'US', 42.29, -71.50, 430),
(1746, 'STANDARD', b'0', 'Holliston', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', '508,774', 'NA', 'US', 42.20, -71.43, 14290),
(1747, 'STANDARD', b'0', 'Hopedale', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', NULL, 'NA', 'US', 42.12, -71.54, 5830),
(1748, 'STANDARD', b'0', 'Hopkinton', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', '508,774', 'NA', 'US', 42.22, -71.52, 16150),
(1749, 'STANDARD', b'0', 'Hudson', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', '508,978', 'NA', 'US', 42.39, -71.56, 18590),
(1752, 'STANDARD', b'0', 'Marlborough', NULL, 'Marlboro', 'MA', 'Middlesex County', 'America/New_York', '508,774,978', 'NA', 'US', 42.34, -71.54, 36750),
(1754, 'STANDARD', b'0', 'Maynard', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', '978', 'NA', 'US', 42.42, -71.45, 9910),
(1756, 'STANDARD', b'0', 'Mendon', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', NULL, 'NA', 'US', 42.10, -71.55, 6060),
(1757, 'STANDARD', b'0', 'Milford', NULL, NULL, 'MA', 'Worcester County', 'America/New_York', '508,774', 'NA', 'US', 42.14, -71.51, 26360),
(1760, 'STANDARD', b'0', 'Natick', NULL, 'N Natick, No Natick, North Natick, S Natick, So Natick, South Natick', 'MA', 'Middlesex County', 'America/New_York', '508,774', 'NA', 'US', 42.28, -71.35, 34350),
(1770, 'STANDARD', b'0', 'Sherborn', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', NULL, 'NA', 'US', 42.23, -71.36, 4310),
(1772, 'STANDARD', b'0', 'Southborough', NULL, 'Southboro', 'MA', 'Worcester County', 'America/New_York', '508,978', 'NA', 'US', 42.30, -71.51, 9970),
(1773, 'STANDARD', b'0', 'Lincoln', NULL, 'Lincoln Center', 'MA', 'Middlesex County', 'America/New_York', '781', 'NA', 'US', 42.41, -71.30, 5340),
(1775, 'STANDARD', b'0', 'Stow', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', NULL, 'NA', 'US', 42.43, -71.50, 7030),
(1776, 'STANDARD', b'0', 'Sudbury', NULL, 'N Sudbury, North Sudbury', 'MA', 'Middlesex County', 'America/New_York', '978', 'NA', 'US', 42.36, -71.40, 18410),
(1778, 'STANDARD', b'0', 'Wayland', NULL, 'Cochituate', 'MA', 'Middlesex County', 'America/New_York', '508,774', 'NA', 'US', 42.36, -71.36, 13910),
(1784, 'PO BOX', b'0', 'Woodville', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', NULL, 'NA', 'US', 42.23, -71.55, 150),
(1801, 'STANDARD', b'0', 'Woburn', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', '339,617,781,978', 'NA', 'US', 42.48, -71.15, 37270),
(1803, 'STANDARD', b'0', 'Burlington', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', '339,781', 'NA', 'US', 42.50, -71.20, 24740),
(1805, 'UNIQUE', b'0', 'Burlington', NULL, 'Lahey Clinic Med Ctr', 'MA', 'Middlesex County', 'America/New_York', '339', 'NA', 'US', 42.50, -71.20, 0),
(1806, 'UNIQUE', b'1', 'Woburn', 'At&t', NULL, 'MA', 'Middlesex County', 'America/New_York', '339', 'NA', 'US', 42.47, -71.15, 0),
(1807, 'UNIQUE', b'0', 'Woburn', NULL, 'National Grid', 'MA', 'Middlesex County', 'America/New_York', '339', 'NA', 'US', 42.48, -71.15, 0),
(1808, 'UNIQUE', b'1', 'Woburn', NULL, 'At&t', 'MA', 'Middlesex County', 'America/New_York', '339', 'NA', 'US', 42.50, -71.12, 0),
(1810, 'STANDARD', b'0', 'Andover', NULL, NULL, 'MA', 'Essex County', 'America/New_York', '508,978', 'NA', 'US', 42.65, -71.14, 33590),
(1812, 'UNIQUE', b'0', 'Andover', NULL, 'Internal Revenue Service', 'MA', 'Essex County', 'America/New_York', '351', 'NA', 'US', 42.65, -71.14, 0),
(1813, 'UNIQUE', b'0', 'Woburn', NULL, 'Mellon Financial Services', 'MA', 'Middlesex County', 'America/New_York', '339', 'NA', 'US', 42.48, -71.15, 0),
(1815, 'UNIQUE', b'0', 'Woburn', NULL, 'Bank Of America', 'MA', 'Middlesex County', 'America/New_York', '339', 'NA', 'US', 42.48, -71.15, 0),
(1821, 'STANDARD', b'0', 'Billerica', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', '508,978', 'NA', 'US', 42.55, -71.26, 29980),
(1822, 'PO BOX', b'0', 'Billerica', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', '351', 'NA', 'US', 42.55, -71.26, 0),
(1824, 'STANDARD', b'0', 'Chelmsford', 'Kates Corner, S Chelmsford', NULL, 'MA', 'Middlesex County', 'America/New_York', NULL, 'NA', 'US', 42.59, -71.36, 25110),
(1826, 'STANDARD', b'0', 'Dracut', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', NULL, 'NA', 'US', 42.68, -71.30, 29730),
(1827, 'STANDARD', b'0', 'Dunstable', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', NULL, 'NA', 'US', 42.66, -71.48, 3120),
(1830, 'STANDARD', b'0', 'Haverhill', NULL, NULL, 'MA', 'Essex County', 'America/New_York', '978', 'NA', 'US', 42.78, -71.08, 21980),
(1831, 'PO BOX', b'0', 'Haverhill', NULL, NULL, 'MA', 'Essex County', 'America/New_York', '351', 'NA', 'US', 42.78, -71.08, 992),
(1832, 'STANDARD', b'0', 'Haverhill', NULL, NULL, 'MA', 'Essex County', 'America/New_York', '978', 'NA', 'US', 42.79, -71.13, 21090),
(1833, 'STANDARD', b'0', 'Georgetown', 'Haverhill', NULL, 'MA', 'Essex County', 'America/New_York', '351,978', 'NA', 'US', 42.73, -70.98, 8180),
(1834, 'STANDARD', b'0', 'Groveland', NULL, NULL, 'MA', 'Essex County', 'America/New_York', NULL, 'NA', 'US', 42.75, -71.03, 6340),
(1835, 'STANDARD', b'0', 'Haverhill', 'Bradford, Ward Hill', NULL, 'MA', 'Essex County', 'America/New_York', '978', 'NA', 'US', 42.75, -71.09, 13130),
(1840, 'STANDARD', b'0', 'Lawrence', NULL, NULL, 'MA', 'Essex County', 'America/New_York', '508,978', 'NA', 'US', 42.71, -71.16, 4200),
(1841, 'STANDARD', b'0', 'Lawrence', NULL, NULL, 'MA', 'Essex County', 'America/New_York', '508,978', 'NA', 'US', 42.71, -71.16, 43940),
(1842, 'PO BOX', b'0', 'Lawrence', NULL, NULL, 'MA', 'Essex County', 'America/New_York', '351', 'NA', 'US', 42.70, -71.16, 1297),
(1843, 'STANDARD', b'0', 'Lawrence', NULL, 'S Lawrence, South Lawrence', 'MA', 'Essex County', 'America/New_York', '508,978', 'NA', 'US', 42.70, -71.16, 23800),
(1844, 'STANDARD', b'0', 'Methuen', NULL, NULL, 'MA', 'Essex County', 'America/New_York', NULL, 'NA', 'US', 42.74, -71.18, 47210),
(1845, 'STANDARD', b'0', 'North Andover', NULL, 'N Andover', 'MA', 'Essex County', 'America/New_York', NULL, 'NA', 'US', 42.70, -71.11, 28000),
(1850, 'STANDARD', b'0', 'Lowell', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', '781,978', 'NA', 'US', 42.66, -71.30, 13690),
(1851, 'STANDARD', b'0', 'Lowell', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', '781,978', 'NA', 'US', 42.63, -71.32, 29150),
(1852, 'STANDARD', b'0', 'Lowell', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', '781,978', 'NA', 'US', 42.63, -71.30, 29580),
(1853, 'PO BOX', b'0', 'Lowell', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', '351', 'NA', 'US', 42.63, -71.32, 1641),
(1854, 'STANDARD', b'0', 'Lowell', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', '978', 'NA', 'US', 42.65, -71.35, 20260),
(1860, 'STANDARD', b'0', 'Merrimac', NULL, NULL, 'MA', 'Essex County', 'America/New_York', '978', 'NA', 'US', 42.83, -71.00, 6110),
(1862, 'STANDARD', b'0', 'North Billerica', 'N Billerica', 'Billerica', 'MA', 'Middlesex County', 'America/New_York', '508,978', 'NA', 'US', 42.58, -71.30, 9070),
(1863, 'STANDARD', b'0', 'North Chelmsford', 'N Chelmsford', NULL, 'MA', 'Middlesex County', 'America/New_York', NULL, 'NA', 'US', 42.63, -71.39, 8740),
(1864, 'STANDARD', b'0', 'North Reading', NULL, 'N Reading', 'MA', 'Middlesex County', 'America/New_York', '978', 'NA', 'US', 42.58, -71.08, 15120),
(1865, 'PO BOX', b'0', 'Nutting Lake', NULL, 'Nuttings Lake', 'MA', 'Middlesex County', 'America/New_York', NULL, 'NA', 'US', 42.54, -71.25, 397),
(1866, 'PO BOX', b'0', 'Pinehurst', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', NULL, 'NA', 'US', 42.53, -71.23, 178),
(1867, 'STANDARD', b'0', 'Reading', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', '781', 'NA', 'US', 42.53, -71.10, 24810),
(1876, 'STANDARD', b'0', 'Tewksbury', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', NULL, 'NA', 'US', 42.61, -71.23, 28550),
(1879, 'STANDARD', b'0', 'Tyngsboro', NULL, 'Tyngsborough', 'MA', 'Middlesex County', 'America/New_York', '978', 'NA', 'US', 42.68, -71.41, 11610),
(1880, 'STANDARD', b'0', 'Wakefield', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', '339,781', 'NA', 'US', 42.50, -71.06, 25180),
(1885, 'PO BOX', b'0', 'West Boxford', NULL, NULL, 'MA', 'Essex County', 'America/New_York', NULL, 'NA', 'US', 42.67, -71.03, 331),
(1886, 'STANDARD', b'0', 'Westford', NULL, 'Forge Village, Nabnasset', 'MA', 'Middlesex County', 'America/New_York', '508,978', 'NA', 'US', 42.58, -71.43, 24190),
(1887, 'STANDARD', b'0', 'Wilmington', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', '978', 'NA', 'US', 42.55, -71.16, 22520),
(1888, 'PO BOX', b'0', 'Woburn', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', '339', 'NA', 'US', 42.48, -71.15, 242),
(1889, 'UNIQUE', b'0', 'North Reading', NULL, 'Massachusetts District, N Reading', 'MA', 'Middlesex County', 'America/New_York', '351', 'NA', 'US', 42.56, -71.06, 0),
(1890, 'STANDARD', b'0', 'Winchester', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', '781', 'NA', 'US', 42.45, -71.14, 22310),
(1899, 'UNIQUE', b'0', 'Andover', NULL, 'Bar Coded I R S', 'MA', 'Essex County', 'America/New_York', '351', 'NA', 'US', 42.65, -71.14, 0),
(1901, 'STANDARD', b'0', 'Lynn', NULL, NULL, 'MA', 'Essex County', 'America/New_York', '339,781', 'NA', 'US', 42.46, -70.95, 1440),
(1902, 'STANDARD', b'0', 'Lynn', NULL, NULL, 'MA', 'Essex County', 'America/New_York', '339,781', 'NA', 'US', 42.47, -70.94, 40210),
(1903, 'PO BOX', b'0', 'Lynn', NULL, NULL, 'MA', 'Essex County', 'America/New_York', '339', 'NA', 'US', 42.47, -70.96, 1107),
(1904, 'STANDARD', b'0', 'Lynn', 'East Lynn', NULL, 'MA', 'Essex County', 'America/New_York', '508,978', 'NA', 'US', 42.47, -70.96, 18340),
(1905, 'STANDARD', b'0', 'Lynn', 'West Lynn', NULL, 'MA', 'Essex County', 'America/New_York', '617,339,781', 'NA', 'US', 42.47, -70.98, 23830),
(1906, 'STANDARD', b'0', 'Saugus', NULL, NULL, 'MA', 'Essex County', 'America/New_York', '339,617,781', 'NA', 'US', 42.46, -71.01, 25580),
(1907, 'STANDARD', b'0', 'Swampscott', NULL, NULL, 'MA', 'Essex County', 'America/New_York', NULL, 'NA', 'US', 42.47, -70.91, 13970),
(1908, 'STANDARD', b'0', 'Nahant', NULL, NULL, 'MA', 'Essex County', 'America/New_York', NULL, 'NA', 'US', 42.43, -70.93, 3200),
(1910, 'UNIQUE', b'0', 'Lynn', NULL, 'General Elec Co', 'MA', 'Essex County', 'America/New_York', '339', 'NA', 'US', 42.47, -70.96, 0),
(1913, 'STANDARD', b'0', 'Amesbury', NULL, NULL, 'MA', 'Essex County', 'America/New_York', '978', 'NA', 'US', 42.85, -70.92, 15190),
(1915, 'STANDARD', b'0', 'Beverly', NULL, 'Beverly Farms', 'MA', 'Essex County', 'America/New_York', '508,978', 'NA', 'US', 42.57, -70.87, 34410),
(1921, 'STANDARD', b'0', 'Boxford', NULL, NULL, 'MA', 'Essex County', 'America/New_York', NULL, 'NA', 'US', 42.67, -70.98, 7940),
(1922, 'STANDARD', b'0', 'Byfield', 'Newbury', NULL, 'MA', 'Essex County', 'America/New_York', NULL, 'NA', 'US', 42.75, -70.93, 3110),
(1923, 'STANDARD', b'0', 'Danvers', NULL, NULL, 'MA', 'Essex County', 'America/New_York', '351,978', 'NA', 'US', 42.57, -70.95, 25830),
(1929, 'STANDARD', b'0', 'Essex', NULL, NULL, 'MA', 'Essex County', 'America/New_York', '978', 'NA', 'US', 42.63, -70.77, 3350),
(1930, 'STANDARD', b'0', 'Gloucester', NULL, 'Magnolia', 'MA', 'Essex County', 'America/New_York', '508,978', 'NA', 'US', 42.62, -70.66, 25490),
(1931, 'PO BOX', b'0', 'Gloucester', NULL, NULL, 'MA', 'Essex County', 'America/New_York', '351', 'NA', 'US', 42.62, -70.66, 554),
(1936, 'PO BOX', b'0', 'Hamilton', NULL, NULL, 'MA', 'Essex County', 'America/New_York', NULL, 'NA', 'US', 42.61, -70.86, 361),
(1937, 'PO BOX', b'0', 'Hathorne', NULL, NULL, 'MA', 'Essex County', 'America/New_York', NULL, 'NA', 'US', 42.59, -70.98, 375),
(1938, 'STANDARD', b'0', 'Ipswich', NULL, NULL, 'MA', 'Essex County', 'America/New_York', '978', 'NA', 'US', 42.67, -70.83, 12860),
(1940, 'STANDARD', b'0', 'Lynnfield', NULL, 'South Lynnfield', 'MA', 'Essex County', 'America/New_York', '781', 'NA', 'US', 42.53, -71.04, 12880),
(1944, 'STANDARD', b'0', 'Manchester', 'Manchester By The Sea', NULL, 'MA', 'Essex County', 'America/New_York', '508,978', 'NA', 'US', 42.56, -70.76, 5110),
(1945, 'STANDARD', b'0', 'Marblehead', NULL, 'Mhead', 'MA', 'Essex County', 'America/New_York', '781', 'NA', 'US', 42.50, -70.85, 20070),
(1949, 'STANDARD', b'0', 'Middleton', NULL, NULL, 'MA', 'Essex County', 'America/New_York', NULL, 'NA', 'US', 42.60, -71.01, 8180),
(1950, 'STANDARD', b'0', 'Newburyport', NULL, 'Plum Island', 'MA', 'Essex County', 'America/New_York', '978', 'NA', 'US', 42.81, -70.88, 16750),
(1951, 'STANDARD', b'0', 'Newbury', 'Newburyport', 'Plum Island', 'MA', 'Essex County', 'America/New_York', '978', 'NA', 'US', 42.77, -70.85, 3390),
(1952, 'STANDARD', b'0', 'Salisbury', 'Salisbury Bch, Salisbury Beach', NULL, 'MA', 'Essex County', 'America/New_York', NULL, 'NA', 'US', 42.83, -70.84, 7300),
(1960, 'STANDARD', b'0', 'Peabody', NULL, 'West Peabody', 'MA', 'Essex County', 'America/New_York', '508,978', 'NA', 'US', 42.53, -70.97, 47880),
(1961, 'PO BOX', b'0', 'Peabody', NULL, NULL, 'MA', 'Essex County', 'America/New_York', '351', 'NA', 'US', 42.53, -70.97, 315),
(1965, 'PO BOX', b'0', 'Prides Crossing', 'Prides Crssng', NULL, 'MA', 'Essex County', 'America/New_York', '351', 'NA', 'US', 42.56, -70.86, 428),
(1966, 'STANDARD', b'0', 'Rockport', NULL, 'Pigeon Cove', 'MA', 'Essex County', 'America/New_York', '508,351,978', 'NA', 'US', 42.64, -70.61, 6380),
(1969, 'STANDARD', b'0', 'Rowley', NULL, NULL, 'MA', 'Essex County', 'America/New_York', '978', 'NA', 'US', 42.72, -70.89, 6010),
(1970, 'STANDARD', b'0', 'Salem', NULL, NULL, 'MA', 'Essex County', 'America/New_York', '508,978', 'NA', 'US', 42.51, -70.90, 36170),
(1971, 'PO BOX', b'0', 'Salem', NULL, NULL, 'MA', 'Essex County', 'America/New_York', '351', 'NA', 'US', 42.51, -70.89, 356),
(1982, 'STANDARD', b'0', 'South Hamilton', 'S Hamilton', 'Hamilton', 'MA', 'Essex County', 'America/New_York', '978', 'NA', 'US', 42.62, -70.86, 6980),
(1983, 'STANDARD', b'0', 'Topsfield', NULL, NULL, 'MA', 'Essex County', 'America/New_York', '978', 'NA', 'US', 42.63, -70.95, 6370),
(1984, 'STANDARD', b'0', 'Wenham', NULL, NULL, 'MA', 'Essex County', 'America/New_York', NULL, 'NA', 'US', 42.60, -70.88, 3710),
(1985, 'STANDARD', b'0', 'West Newbury', NULL, NULL, 'MA', 'Essex County', 'America/New_York', '978', 'NA', 'US', 42.80, -71.00, 4380),
(2018, 'PO BOX', b'0', 'Accord', 'Hingham', 'Norwell', 'MA', 'Plymouth County', 'America/New_York', '339', 'NA', 'US', 42.17, -70.88, 123),
(2019, 'STANDARD', b'0', 'Bellingham', NULL, NULL, 'MA', 'Norfolk County', 'America/New_York', '508,774', 'NA', 'US', 42.09, -71.47, 15890),
(2020, 'PO BOX', b'0', 'Brant Rock', NULL, 'Marshfield', 'MA', 'Plymouth County', 'America/New_York', NULL, 'NA', 'US', 42.08, -70.64, 759),
(2021, 'STANDARD', b'0', 'Canton', NULL, NULL, 'MA', 'Norfolk County', 'America/New_York', '339,781', 'NA', 'US', 42.15, -71.13, 21930),
(2025, 'STANDARD', b'0', 'Cohasset', NULL, NULL, 'MA', 'Norfolk County', 'America/New_York', '339,781', 'NA', 'US', 42.23, -70.80, 8100),
(2026, 'STANDARD', b'0', 'Dedham', NULL, NULL, 'MA', 'Norfolk County', 'America/New_York', '617,781', 'NA', 'US', 42.24, -71.17, 23210),
(2027, 'PO BOX', b'0', 'Dedham', NULL, NULL, 'MA', 'Norfolk County', 'America/New_York', '339', 'NA', 'US', 42.24, -71.17, 183),
(2030, 'STANDARD', b'0', 'Dover', NULL, NULL, 'MA', 'Norfolk County', 'America/New_York', '508,774', 'NA', 'US', 42.24, -71.27, 5800),
(2031, 'STANDARD', b'1', 'East Mansfield', 'Mansfield, E Mansfield', NULL, 'MA', 'Bristol County', 'America/New_York', '508', 'NA', 'US', 42.02, -71.17, 0),
(2032, 'STANDARD', b'0', 'East Walpole', NULL, 'E Walpole, Walpole', 'MA', 'Norfolk County', 'America/New_York', '508,774,339,781', 'NA', 'US', 42.15, -71.21, 4580),
(2035, 'STANDARD', b'0', 'Foxboro', 'Foxborough', NULL, 'MA', 'Norfolk County', 'America/New_York', '781,508,774', 'NA', 'US', 42.06, -71.24, 16830),
(2038, 'STANDARD', b'0', 'Franklin', NULL, NULL, 'MA', 'Norfolk County', 'America/New_York', '508,774', 'NA', 'US', 42.08, -71.38, 30920),
(2040, 'PO BOX', b'0', 'Greenbush', 'Scituate', NULL, 'MA', 'Plymouth County', 'America/New_York', '339', 'NA', 'US', 42.19, -70.76, 122),
(2041, 'PO BOX', b'0', 'Green Harbor', NULL, 'Marshfield', 'MA', 'Plymouth County', 'America/New_York', NULL, 'NA', 'US', 42.10, -70.71, 843),
(2043, 'STANDARD', b'0', 'Hingham', NULL, 'Accord', 'MA', 'Plymouth County', 'America/New_York', '339,781', 'NA', 'US', 42.23, -70.88, 23080),
(2044, 'UNIQUE', b'0', 'Hingham', NULL, 'Shared Firm Zip Code', 'MA', 'Plymouth County', 'America/New_York', '339', 'NA', 'US', 42.23, -70.88, 0),
(2045, 'STANDARD', b'0', 'Hull', NULL, 'Nantasket Beach', 'MA', 'Plymouth County', 'America/New_York', '781', 'NA', 'US', 42.30, -70.90, 9080),
(2047, 'PO BOX', b'0', 'Humarock', NULL, 'Marshfield', 'MA', 'Plymouth County', 'America/New_York', '339', 'NA', 'US', 42.13, -70.69, 649),
(2048, 'STANDARD', b'0', 'Mansfield', NULL, NULL, 'MA', 'Bristol County', 'America/New_York', '508,774', 'NA', 'US', 42.02, -71.21, 23100),
(2050, 'STANDARD', b'0', 'Marshfield', NULL, NULL, 'MA', 'Plymouth County', 'America/New_York', '339,781', 'NA', 'US', 42.09, -70.70, 22550),
(2051, 'PO BOX', b'0', 'Marshfield Hills', 'Marshfld Hls', 'Marshfield', 'MA', 'Plymouth County', 'America/New_York', NULL, 'NA', 'US', 42.15, -70.73, 752),
(2052, 'STANDARD', b'0', 'Medfield', NULL, NULL, 'MA', 'Norfolk County', 'America/New_York', '508,774', 'NA', 'US', 42.18, -71.30, 12690),
(2053, 'STANDARD', b'0', 'Medway', NULL, NULL, 'MA', 'Norfolk County', 'America/New_York', '508,774', 'NA', 'US', 42.16, -71.43, 12820),
(2054, 'STANDARD', b'0', 'Millis', NULL, NULL, 'MA', 'Norfolk County', 'America/New_York', '508,774', 'NA', 'US', 42.16, -71.35, 7840),
(2055, 'PO BOX', b'0', 'Minot', 'Scituate', NULL, 'MA', 'Plymouth County', 'America/New_York', '339', 'NA', 'US', 42.19, -70.76, 77),
(2056, 'STANDARD', b'0', 'Norfolk', NULL, NULL, 'MA', 'Norfolk County', 'America/New_York', NULL, 'NA', 'US', 42.11, -71.31, 9820),
(2059, 'PO BOX', b'0', 'North Marshfield', 'N Marshfield', 'Marshfield', 'MA', 'Plymouth County', 'America/New_York', NULL, 'NA', 'US', 42.10, -70.71, 363),
(2060, 'PO BOX', b'0', 'North Scituate', 'N Scituate, Scituate', NULL, 'MA', 'Plymouth County', 'America/New_York', '339', 'NA', 'US', 42.21, -70.76, 237),
(2061, 'STANDARD', b'0', 'Norwell', NULL, NULL, 'MA', 'Plymouth County', 'America/New_York', '339,781', 'NA', 'US', 42.16, -70.78, 10930),
(2062, 'STANDARD', b'0', 'Norwood', NULL, NULL, 'MA', 'Norfolk County', 'America/New_York', '339,781', 'NA', 'US', 42.18, -71.19, 27500),
(2065, 'PO BOX', b'0', 'Ocean Bluff', 'Marshfield', NULL, 'MA', 'Plymouth County', 'America/New_York', NULL, 'NA', 'US', 42.10, -70.71, 447),
(2066, 'STANDARD', b'0', 'Scituate', NULL, 'Scituate Center, Scituate Harbor', 'MA', 'Plymouth County', 'America/New_York', '339,781', 'NA', 'US', 42.18, -70.73, 17470),
(2067, 'STANDARD', b'0', 'Sharon', NULL, NULL, 'MA', 'Norfolk County', 'America/New_York', '339,781', 'NA', 'US', 42.11, -71.18, 18210),
(2070, 'PO BOX', b'0', 'Sheldonville', NULL, 'Wrentham', 'MA', 'Norfolk County', 'America/New_York', NULL, 'NA', 'US', 42.05, -71.35, 182),
(2071, 'STANDARD', b'0', 'South Walpole', NULL, 'S Walpole, Walpole', 'MA', 'Norfolk County', 'America/New_York', '508,774,781', 'NA', 'US', 42.10, -71.27, 1010),
(2072, 'STANDARD', b'0', 'Stoughton', NULL, NULL, 'MA', 'Norfolk County', 'America/New_York', '781', 'NA', 'US', 42.11, -71.10, 26350),
(2081, 'STANDARD', b'0', 'Walpole', NULL, NULL, 'MA', 'Norfolk County', 'America/New_York', '781,508,774', 'NA', 'US', 42.13, -71.24, 18890),
(2090, 'STANDARD', b'0', 'Westwood', NULL, 'Islington', 'MA', 'Norfolk County', 'America/New_York', '339,617,781', 'NA', 'US', 42.21, -71.21, 15350),
(2093, 'STANDARD', b'0', 'Wrentham', NULL, NULL, 'MA', 'Norfolk County', 'America/New_York', '508,774', 'NA', 'US', 42.06, -71.33, 10820),
(2108, 'STANDARD', b'0', 'Boston', NULL, NULL, 'MA', 'Suffolk County', 'America/New_York', '617,857,339', 'NA', 'US', 42.36, -71.06, 4050),
(2109, 'STANDARD', b'0', 'Boston', NULL, NULL, 'MA', 'Suffolk County', 'America/New_York', '617', 'NA', 'US', 42.37, -71.05, 3980),
(2110, 'STANDARD', b'0', 'Boston', NULL, NULL, 'MA', 'Suffolk County', 'America/New_York', '508,774,617,781,857,978', 'NA', 'US', 42.36, -71.05, 3320),
(2111, 'STANDARD', b'0', 'Boston', NULL, NULL, 'MA', 'Suffolk County', 'America/New_York', '617,781,978,339,857', 'NA', 'US', 42.35, -71.06, 5720),
(2112, 'PO BOX', b'0', 'Boston', NULL, NULL, 'MA', 'Suffolk County', 'America/New_York', '617', 'NA', 'US', 42.35, -71.06, 574),
(2113, 'STANDARD', b'0', 'Boston', NULL, NULL, 'MA', 'Suffolk County', 'America/New_York', '781,617', 'NA', 'US', 42.37, -71.06, 5320),
(2114, 'STANDARD', b'0', 'Boston', NULL, NULL, 'MA', 'Suffolk County', 'America/New_York', '617,339,857', 'NA', 'US', 42.36, -71.07, 9350),
(2115, 'STANDARD', b'0', 'Boston', NULL, NULL, 'MA', 'Suffolk County', 'America/New_York', '617,857', 'NA', 'US', 42.34, -71.10, 9810),
(2116, 'STANDARD', b'0', 'Boston', NULL, NULL, 'MA', 'Suffolk County', 'America/New_York', '508,617,781,978,857', 'NA', 'US', 42.35, -71.08, 15420),
(2117, 'PO BOX', b'0', 'Boston', NULL, NULL, 'MA', 'Suffolk County', 'America/New_York', '617', 'NA', 'US', 42.35, -71.06, 443),
(2118, 'STANDARD', b'0', 'Boston', 'Roxbury', NULL, 'MA', 'Suffolk County', 'America/New_York', '857,617', 'NA', 'US', 42.34, -71.07, 20190),
(2119, 'STANDARD', b'0', 'Roxbury', 'Boston', NULL, 'MA', 'Suffolk County', 'America/New_York', '617,857', 'NA', 'US', 42.32, -71.09, 22430),
(2120, 'STANDARD', b'0', 'Roxbury Crossing', 'Boston, Mission Hill, Roxbury, Roxbury Xing', NULL, 'MA', 'Suffolk County', 'America/New_York', '857,617', 'NA', 'US', 42.33, -71.10, 7730),
(2121, 'STANDARD', b'0', 'Dorchester', 'Boston, Grove Hall', NULL, 'MA', 'Suffolk County', 'America/New_York', '617,781,857', 'NA', 'US', 42.31, -71.09, 23300),
(2122, 'STANDARD', b'0', 'Dorchester', 'Boston', NULL, 'MA', 'Suffolk County', 'America/New_York', '781,617,857', 'NA', 'US', 42.29, -71.04, 21000),
(2123, 'PO BOX', b'0', 'Boston', NULL, NULL, 'MA', 'Suffolk County', 'America/New_York', '617', 'NA', 'US', 42.35, -71.06, 449),
(2124, 'STANDARD', b'0', 'Dorchester Center', 'Boston, Dorchester, Dorchestr Ctr', NULL, 'MA', 'Suffolk County', 'America/New_York', '617', 'NA', 'US', 42.29, -71.07, 43850),
(2125, 'STANDARD', b'0', 'Dorchester', 'Boston, Uphams Corner', NULL, 'MA', 'Suffolk County', 'America/New_York', '617,857,781', 'NA', 'US', 42.32, -71.06, 27860),
(2126, 'STANDARD', b'0', 'Mattapan', 'Boston', 'Hyde Park', 'MA', 'Suffolk County', 'America/New_York', '617', 'NA', 'US', 42.27, -71.10, 20900),
(2127, 'STANDARD', b'0', 'Boston', 'South Boston', 'S Boston', 'MA', 'Suffolk County', 'America/New_York', '617,774,781,978,857', 'NA', 'US', 42.33, -71.04, 29000),
(2128, 'STANDARD', b'0', 'Boston', 'East Boston', 'E Boston', 'MA', 'Suffolk County', 'America/New_York', '617,857', 'NA', 'US', 42.36, -71.01, 34440),
(2129, 'STANDARD', b'0', 'Charlestown', 'Boston', NULL, 'MA', 'Suffolk County', 'America/New_York', '857,617,781', 'NA', 'US', 42.38, -71.06, 15910),
(2130, 'STANDARD', b'0', 'Jamaica Plain', 'Boston', NULL, 'MA', 'Suffolk County', 'America/New_York', '617,857', 'NA', 'US', 42.30, -71.11, 30590),
(2131, 'STANDARD', b'0', 'Roslindale', 'Boston', NULL, 'MA', 'Suffolk County', 'America/New_York', '617', 'NA', 'US', 42.28, -71.13, 27800),
(2132, 'STANDARD', b'0', 'West Roxbury', 'Boston', 'W Roxbury', 'MA', 'Suffolk County', 'America/New_York', '617,781', 'NA', 'US', 42.28, -71.16, 25130),
(2133, 'STANDARD', b'0', 'Boston', NULL, NULL, 'MA', 'Suffolk County', 'America/New_York', '339,508,781,857,978,617', 'NA', 'US', 42.35, -71.06, 0),
(2134, 'STANDARD', b'0', 'Allston', 'Boston', NULL, 'MA', 'Suffolk County', 'America/New_York', '617,857', 'NA', 'US', 42.35, -71.13, 13390),
(2135, 'STANDARD', b'0', 'Brighton', 'Boston', NULL, 'MA', 'Suffolk County', 'America/New_York', '617,857', 'NA', 'US', 42.35, -71.15, 31660),
(2136, 'STANDARD', b'0', 'Hyde Park', 'Boston, Readville', NULL, 'MA', 'Suffolk County', 'America/New_York', '617,857', 'NA', 'US', 42.26, -71.13, 32400),
(2137, 'PO BOX', b'0', 'Readville', 'Boston, Hyde Park', NULL, 'MA', 'Suffolk County', 'America/New_York', '617', 'NA', 'US', 42.25, -71.12, 180),
(2138, 'STANDARD', b'0', 'Cambridge', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', '508,617,857', 'NA', 'US', 42.38, -71.14, 22390),
(2139, 'STANDARD', b'0', 'Cambridge', NULL, 'Cambridgeport, Inman Square', 'MA', 'Middlesex County', 'America/New_York', '339,781,978,508,617,857', 'NA', 'US', 42.37, -71.11, 26890),
(2140, 'STANDARD', b'0', 'Cambridge', 'N Cambridge, North Cambridge', 'Porter Square', 'MA', 'Middlesex County', 'America/New_York', '617', 'NA', 'US', 42.39, -71.13, 16870),
(2141, 'STANDARD', b'0', 'Cambridge', 'E Cambridge, East Cambridge', NULL, 'MA', 'Middlesex County', 'America/New_York', '339,617,508,781,857,978', 'NA', 'US', 42.37, -71.08, 9740),
(2142, 'STANDARD', b'0', 'Cambridge', NULL, 'Kendall Square', 'MA', 'Middlesex County', 'America/New_York', '508,781,857,978,339,617', 'NA', 'US', 42.36, -71.08, 2420),
(2143, 'STANDARD', b'0', 'Somerville', NULL, 'E Somerville, East Somerville', 'MA', 'Middlesex County', 'America/New_York', '617,857', 'NA', 'US', 42.38, -71.10, 20890),
(2144, 'STANDARD', b'0', 'Somerville', 'W Somerville, West Somerville', NULL, 'MA', 'Middlesex County', 'America/New_York', '617,857', 'NA', 'US', 42.40, -71.12, 20290),
(2145, 'STANDARD', b'0', 'Somerville', 'Winter Hill', NULL, 'MA', 'Middlesex County', 'America/New_York', '617,857', 'NA', 'US', 42.39, -71.10, 22430),
(2148, 'STANDARD', b'0', 'Malden', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', '857,339,617,781', 'NA', 'US', 42.43, -71.05, 54800),
(2149, 'STANDARD', b'0', 'Everett', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', '617,857', 'NA', 'US', 42.40, -71.05, 39470),
(2150, 'STANDARD', b'0', 'Chelsea', NULL, NULL, 'MA', 'Suffolk County', 'America/New_York', '617,857', 'NA', 'US', 42.39, -71.03, 32330),
(2151, 'STANDARD', b'0', 'Revere', NULL, 'Beachmont, Revere Beach', 'MA', 'Suffolk County', 'America/New_York', '617,339,781', 'NA', 'US', 42.41, -70.99, 47170),
(2152, 'STANDARD', b'0', 'Winthrop', NULL, NULL, 'MA', 'Suffolk County', 'America/New_York', '617,857', 'NA', 'US', 42.37, -70.98, 15680),
(2153, 'PO BOX', b'0', 'Medford', 'Tufts Univ, Tufts University', NULL, 'MA', 'Middlesex County', 'America/New_York', '617', 'NA', 'US', 42.42, -71.10, 0),
(2155, 'STANDARD', b'0', 'Medford', NULL, 'W Medford, West Medford', 'MA', 'Middlesex County', 'America/New_York', '617,339,781', 'NA', 'US', 42.42, -71.10, 50390),
(2156, 'PO BOX', b'0', 'West Medford', NULL, 'Medford, W Medford', 'MA', 'Middlesex County', 'America/New_York', '339', 'NA', 'US', 42.42, -71.13, 20),
(2163, 'STANDARD', b'0', 'Boston', 'Cambridge', 'Soldiers Field', 'MA', 'Suffolk County', 'America/New_York', '339,781,978,508,617,857', 'NA', 'US', 42.37, -71.12, 630),
(2169, 'STANDARD', b'0', 'Quincy', NULL, 'Houghs Neck, Quincy Center, South Quincy, West Quincy', 'MA', 'Norfolk County', 'America/New_York', '617,857', 'NA', 'US', 42.26, -71.00, 49210),
(2170, 'STANDARD', b'0', 'Quincy', 'Wollaston', NULL, 'MA', 'Norfolk County', 'America/New_York', '617,857', 'NA', 'US', 42.27, -71.02, 18260),
(2171, 'STANDARD', b'0', 'Quincy', 'North Quincy, Squantum', 'Marina Bay, N Quincy, No Quincy, Norfolk Downs', 'MA', 'Norfolk County', 'America/New_York', '617,857', 'NA', 'US', 42.29, -71.02, 16680),
(2176, 'STANDARD', b'0', 'Melrose', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', '339,617,781', 'NA', 'US', 42.45, -71.05, 26840),
(2180, 'STANDARD', b'0', 'Stoneham', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', '617,781', 'NA', 'US', 42.47, -71.09, 21130),
(2184, 'STANDARD', b'0', 'Braintree', NULL, 'Braintree Highlands, Braintree Hld, E Braintree, East Braintree', 'MA', 'Norfolk County', 'America/New_York', '339,617,781', 'NA', 'US', 42.20, -71.00, 36000),
(2185, 'PO BOX', b'0', 'Braintree', NULL, 'Braintree Phantom', 'MA', 'Norfolk County', 'America/New_York', '339', 'NA', 'US', 42.20, -71.00, 257),
(2186, 'STANDARD', b'0', 'Milton', NULL, 'East Milton', 'MA', 'Norfolk County', 'America/New_York', '617', 'NA', 'US', 42.24, -71.08, 26280),
(2187, 'PO BOX', b'0', 'Milton Village', 'Milton Vlg', NULL, 'MA', 'Norfolk County', 'America/New_York', '617', 'NA', 'US', 42.26, -71.08, 63),
(2188, 'STANDARD', b'0', 'Weymouth', NULL, 'Weymouth Lndg', 'MA', 'Norfolk County', 'America/New_York', '339,617,781', 'NA', 'US', 42.20, -70.96, 13740),
(2189, 'STANDARD', b'0', 'East Weymouth', 'Weymouth', NULL, 'MA', 'Norfolk County', 'America/New_York', '339,617,781', 'NA', 'US', 42.20, -70.94, 13430),
(2190, 'STANDARD', b'0', 'South Weymouth', 'S Weymouth, Weymouth', 'Weymouth Nas', 'MA', 'Norfolk County', 'America/New_York', NULL, 'NA', 'US', 42.17, -70.95, 16180),
(2191, 'STANDARD', b'0', 'North Weymouth', 'N Weymouth, Weymouth', NULL, 'MA', 'Norfolk County', 'America/New_York', '339,617,781', 'NA', 'US', 42.24, -70.94, 7750),
(2196, 'PO BOX', b'0', 'Boston', NULL, NULL, 'MA', 'Suffolk County', 'America/New_York', '617', 'NA', 'US', 42.35, -71.06, 793),
(2199, 'STANDARD', b'0', 'Boston', NULL, NULL, 'MA', 'Suffolk County', 'America/New_York', '339,857,978,508,617,781', 'NA', 'US', 42.35, -71.08, 1290),
(2201, 'UNIQUE', b'0', 'Boston', NULL, 'Boston City Hall', 'MA', 'Suffolk County', 'America/New_York', '617', 'NA', 'US', 42.35, -71.06, 15),
(2203, 'STANDARD', b'0', 'Boston', NULL, NULL, 'MA', 'Suffolk County', 'America/New_York', '857', 'NA', 'US', 42.36, -71.06, 0),
(2204, 'UNIQUE', b'0', 'Boston', NULL, 'Mass Tax, Massachusetts Tax', 'MA', 'Suffolk County', 'America/New_York', '617', 'NA', 'US', 42.35, -71.06, 0),
(2205, 'PO BOX', b'0', 'Boston', NULL, NULL, 'MA', 'Suffolk County', 'America/New_York', '617', 'NA', 'US', 42.35, -71.06, 1678),
(2206, 'UNIQUE', b'0', 'Boston', NULL, 'State Street Corporation', 'MA', 'Suffolk County', 'America/New_York', '617', 'NA', 'US', 42.35, -71.06, 0),
(2207, 'UNIQUE', b'1', 'Boston', NULL, 'Bell Atlantic Telephone Co', 'MA', 'Suffolk County', 'America/New_York', '617', 'NA', 'US', 42.35, -71.05, 0),
(2210, 'STANDARD', b'0', 'Boston', NULL, NULL, 'MA', 'Suffolk County', 'America/New_York', '617,774,781,857,978', 'NA', 'US', 42.35, -71.04, 3140),
(2211, 'UNIQUE', b'0', 'Boston', NULL, 'Bank Of America', 'MA', 'Suffolk County', 'America/New_York', '617', 'NA', 'US', 42.35, -71.06, 0),
(2212, 'UNIQUE', b'0', 'Boston', NULL, 'Bank Of America', 'MA', 'Suffolk County', 'America/New_York', '617', 'NA', 'US', 42.35, -71.06, 0),
(2215, 'STANDARD', b'0', 'Boston', NULL, 'Boston University, Kenmore', 'MA', 'Suffolk County', 'America/New_York', '617,857', 'NA', 'US', 42.35, -71.10, 7350),
(2216, 'UNIQUE', b'1', 'Boston', NULL, 'John Hancock P O Box 192', 'MA', 'Suffolk County', 'America/New_York', '617', 'NA', 'US', 42.34, -71.07, 17),
(2217, 'UNIQUE', b'0', 'Boston', NULL, 'John Hancock P O Box 505', 'MA', 'Suffolk County', 'America/New_York', '617', 'NA', 'US', 42.35, -71.06, 0),
(2222, 'STANDARD', b'0', 'Boston', NULL, NULL, 'MA', 'Suffolk County', 'America/New_York', '857,617,781', 'NA', 'US', 42.35, -71.06, 0),
(2228, 'PO BOX', b'0', 'East Boston', 'Boston', NULL, 'MA', 'Suffolk County', 'America/New_York', '617', 'NA', 'US', 42.35, -71.06, 93),
(2238, 'PO BOX', b'0', 'Cambridge', 'Harvard Sq, Harvard Square', NULL, 'MA', 'Middlesex County', 'America/New_York', '617', 'NA', 'US', 42.37, -71.11, 534),
(2239, 'UNIQUE', b'1', 'Cambridge', 'Com/energy Services', NULL, 'MA', 'Middlesex County', 'America/New_York', '617', 'NA', 'US', 42.36, -71.10, 0),
(2241, 'UNIQUE', b'0', 'Boston', NULL, 'Bank Of America, Fleet Bank Boston', 'MA', 'Suffolk County', 'America/New_York', '617', 'NA', 'US', 42.35, -71.06, 0),
(2266, 'UNIQUE', b'0', 'Boston', NULL, 'Boston Financial Data Servic', 'MA', 'Suffolk County', 'America/New_York', '617', 'NA', 'US', 42.35, -71.06, 0),
(2269, 'PO BOX', b'0', 'Quincy', NULL, NULL, 'MA', 'Norfolk County', 'America/New_York', '617', 'NA', 'US', 42.26, -71.00, 478),
(2283, 'STANDARD', b'0', 'Boston', NULL, NULL, 'MA', 'Suffolk County', 'America/New_York', '617,781,978,508,857', 'NA', 'US', 42.35, -71.06, 0),
(2284, 'STANDARD', b'0', 'Boston', NULL, NULL, 'MA', 'Suffolk County', 'America/New_York', '508,617,781,857,978', 'NA', 'US', 42.35, -71.06, 0),
(2293, 'UNIQUE', b'0', 'Boston', NULL, 'Fidelity Service Company', 'MA', 'Suffolk County', 'America/New_York', '617', 'NA', 'US', 42.35, -71.06, 0),
(2295, 'UNIQUE', b'0', 'Boston', NULL, 'John Hancock Mutual Ins', 'MA', 'Suffolk County', 'America/New_York', '617', 'NA', 'US', 42.35, -71.06, 0),
(2297, 'UNIQUE', b'0', 'Boston', NULL, 'Cash Management', 'MA', 'Suffolk County', 'America/New_York', '617', 'NA', 'US', 42.35, -71.06, 0),
(2298, 'PO BOX', b'0', 'Boston', NULL, NULL, 'MA', 'Suffolk County', 'America/New_York', '617', 'NA', 'US', 42.34, -71.05, 0),
(2301, 'STANDARD', b'0', 'Brockton', NULL, NULL, 'MA', 'Plymouth County', 'America/New_York', '508,774,781', 'NA', 'US', 42.08, -71.02, 56300),
(2302, 'STANDARD', b'0', 'Brockton', NULL, NULL, 'MA', 'Plymouth County', 'America/New_York', '508,774,781', 'NA', 'US', 42.09, -71.00, 30790),
(2303, 'PO BOX', b'0', 'Brockton', NULL, NULL, 'MA', 'Plymouth County', 'America/New_York', '508', 'NA', 'US', 42.08, -71.02, 1583),
(2304, 'PO BOX', b'0', 'Brockton', NULL, NULL, 'MA', 'Plymouth County', 'America/New_York', '508', 'NA', 'US', 42.08, -71.02, 323),
(2305, 'PO BOX', b'0', 'Brockton', NULL, NULL, 'MA', 'Plymouth County', 'America/New_York', '508', 'NA', 'US', 42.08, -71.02, 464),
(2322, 'STANDARD', b'0', 'Avon', NULL, NULL, 'MA', 'Norfolk County', 'America/New_York', NULL, 'NA', 'US', 42.13, -71.05, 4220),
(2324, 'STANDARD', b'0', 'Bridgewater', NULL, NULL, 'MA', 'Plymouth County', 'America/New_York', '508,774', 'NA', 'US', 41.98, -70.97, 21560),
(2325, 'UNIQUE', b'0', 'Bridgewater', NULL, 'Bridgewater State College', 'MA', 'Plymouth County', 'America/New_York', '508', 'NA', 'US', 41.98, -70.97, 10),
(2327, 'PO BOX', b'0', 'Bryantville', NULL, 'Pembroke', 'MA', 'Plymouth County', 'America/New_York', NULL, 'NA', 'US', 42.06, -70.80, 613),
(2330, 'STANDARD', b'0', 'Carver', NULL, NULL, 'MA', 'Plymouth County', 'America/New_York', '508,774', 'NA', 'US', 41.88, -70.76, 10140),
(2331, 'PO BOX', b'0', 'Duxbury', NULL, NULL, 'MA', 'Plymouth County', 'America/New_York', '339', 'NA', 'US', 42.04, -70.67, 1883),
(2332, 'STANDARD', b'0', 'Duxbury', NULL, NULL, 'MA', 'Plymouth County', 'America/New_York', '781', 'NA', 'US', 42.04, -70.67, 14010),
(2333, 'STANDARD', b'0', 'East Bridgewater', 'E Bridgewater, E Bridgewtr', NULL, 'MA', 'Plymouth County', 'America/New_York', '508,774', 'NA', 'US', 42.03, -70.95, 13540),
(2334, 'PO BOX', b'0', 'Easton', NULL, NULL, 'MA', 'Bristol County', 'America/New_York', '508', 'NA', 'US', 42.03, -71.10, 375),
(2337, 'PO BOX', b'0', 'Elmwood', NULL, 'East Bridgewater', 'MA', 'Plymouth County', 'America/New_York', NULL, 'NA', 'US', 42.01, -70.96, 119),
(2338, 'STANDARD', b'0', 'Halifax', NULL, NULL, 'MA', 'Plymouth County', 'America/New_York', '339,781', 'NA', 'US', 41.98, -70.86, 7260),
(2339, 'STANDARD', b'0', 'Hanover', NULL, 'Assinippi, West Hanover', 'MA', 'Plymouth County', 'America/New_York', '781', 'NA', 'US', 42.11, -70.81, 14110),
(2340, 'UNIQUE', b'0', 'Hanover', NULL, 'Wearguard', 'MA', 'Plymouth County', 'America/New_York', '339', 'NA', 'US', 42.11, -70.81, 0),
(2341, 'STANDARD', b'0', 'Hanson', NULL, NULL, 'MA', 'Plymouth County', 'America/New_York', '339,781', 'NA', 'US', 42.06, -70.85, 9710),
(2343, 'STANDARD', b'0', 'Holbrook', NULL, NULL, 'MA', 'Norfolk County', 'America/New_York', NULL, 'NA', 'US', 42.14, -71.00, 10240),
(2344, 'UNIQUE', b'0', 'Middleboro', 'Middleborough', 'Aetna Life & Casualty Co', 'MA', 'Plymouth County', 'America/New_York', '508', 'NA', 'US', 41.86, -70.90, 0),
(2345, 'PO BOX', b'0', 'Manomet', NULL, 'Plymouth', 'MA', 'Plymouth County', 'America/New_York', NULL, 'NA', 'US', 41.92, -70.56, 1470),
(2346, 'STANDARD', b'0', 'Middleboro', NULL, 'Middleborough', 'MA', 'Plymouth County', 'America/New_York', '508,774', 'NA', 'US', 41.86, -70.90, 21370),
(2347, 'STANDARD', b'0', 'Lakeville', NULL, 'Middleboro', 'MA', 'Plymouth County', 'America/New_York', '774', 'NA', 'US', 41.85, -70.95, 10570),
(2348, 'UNIQUE', b'0', 'Lakeville', 'Middleboro, Middleborough', 'Lakeville Phantom, Talbots', 'MA', 'Plymouth County', 'America/New_York', '508', 'NA', 'US', 41.86, -70.90, 0),
(2349, 'UNIQUE', b'0', 'Middleboro', 'Middleborough', 'Ocean Spray', 'MA', 'Plymouth County', 'America/New_York', '508', 'NA', 'US', 41.86, -70.90, 0),
(2350, 'PO BOX', b'0', 'Monponsett', NULL, 'Hanson', 'MA', 'Plymouth County', 'America/New_York', NULL, 'NA', 'US', 42.02, -70.84, 494),
(2351, 'STANDARD', b'0', 'Abington', NULL, NULL, 'MA', 'Plymouth County', 'America/New_York', NULL, 'NA', 'US', 42.11, -70.95, 15300),
(2355, 'PO BOX', b'0', 'North Carver', NULL, 'Carver, East Carver', 'MA', 'Plymouth County', 'America/New_York', NULL, 'NA', 'US', 41.91, -70.79, 409),
(2356, 'STANDARD', b'0', 'North Easton', NULL, 'Easton, N Easton, No Easton', 'MA', 'Bristol County', 'America/New_York', '508,774', 'NA', 'US', 42.05, -71.10, 12070),
(2357, 'UNIQUE', b'0', 'North Easton', 'Stonehill Col, Stonehill College', 'Easton, Stonehill Coll', 'MA', 'Bristol County', 'America/New_York', '508', 'NA', 'US', 42.06, -71.08, 14),
(2358, 'PO BOX', b'0', 'North Pembroke', 'N Pembroke', 'Pembroke', 'MA', 'Plymouth County', 'America/New_York', NULL, 'NA', 'US', 42.09, -70.78, 282),
(2359, 'STANDARD', b'0', 'Pembroke', NULL, 'East Pembroke', 'MA', 'Plymouth County', 'America/New_York', '339,781', 'NA', 'US', 42.06, -70.80, 16950),
(2360, 'STANDARD', b'0', 'Plymouth', NULL, 'Cedarville', 'MA', 'Plymouth County', 'America/New_York', '508,774', 'NA', 'US', 41.95, -70.66, 50690),
(2361, 'PO BOX', b'0', 'Plymouth', NULL, NULL, 'MA', 'Plymouth County', 'America/New_York', '508', 'NA', 'US', 41.95, -70.66, 252),
(2362, 'PO BOX', b'0', 'Plymouth', NULL, NULL, 'MA', 'Plymouth County', 'America/New_York', '508', 'NA', 'US', 41.95, -70.66, 981),
(2364, 'STANDARD', b'0', 'Kingston', NULL, 'Rocky Nook, Silver Lake', 'MA', 'Plymouth County', 'America/New_York', '339,617,781', 'NA', 'US', 41.99, -70.71, 12300),
(2366, 'PO BOX', b'0', 'South Carver', NULL, 'Carver', 'MA', 'Plymouth County', 'America/New_York', NULL, 'NA', 'US', 41.85, -70.66, 385),
(2367, 'STANDARD', b'0', 'Plympton', NULL, NULL, 'MA', 'Plymouth County', 'America/New_York', NULL, 'NA', 'US', 41.95, -70.81, 2730),
(2368, 'STANDARD', b'0', 'Randolph', NULL, NULL, 'MA', 'Norfolk County', 'America/New_York', '339,781', 'NA', 'US', 42.17, -71.05, 31010),
(2370, 'STANDARD', b'0', 'Rockland', NULL, NULL, 'MA', 'Plymouth County', 'America/New_York', '339,781', 'NA', 'US', 42.13, -70.91, 16580),
(2375, 'STANDARD', b'0', 'South Easton', NULL, 'Easton, S Easton, So Easton', 'MA', 'Bristol County', 'America/New_York', '508,774', 'NA', 'US', 42.03, -71.10, 9360),
(2379, 'STANDARD', b'0', 'West Bridgewater', 'W Bridgewater', NULL, 'MA', 'Plymouth County', 'America/New_York', '508', 'NA', 'US', 42.01, -71.00, 6800),
(2381, 'PO BOX', b'0', 'White Horse Beach', 'Wht Horse Bch', 'Plymouth', 'MA', 'Plymouth County', 'America/New_York', NULL, 'NA', 'US', 41.93, -70.59, 383),
(2382, 'STANDARD', b'0', 'Whitman', NULL, NULL, 'MA', 'Plymouth County', 'America/New_York', '781', 'NA', 'US', 42.08, -70.93, 14110);
INSERT INTO `zips` (`zip`, `type`, `decommissioned`, `primary_city`, `acceptable_cities`, `unacceptable_cities`, `state`, `county`, `timezone`, `area_codes`, `world_region`, `country`, `latitude`, `longitude`, `irs_estimated_population_2015`) VALUES
(2420, 'STANDARD', b'0', 'Lexington', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', '339,781', 'NA', 'US', 42.46, -71.22, 14980),
(2421, 'STANDARD', b'0', 'Lexington', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', '339,781', 'NA', 'US', 42.44, -71.23, 17770),
(2445, 'STANDARD', b'0', 'Brookline', NULL, NULL, 'MA', 'Norfolk County', 'America/New_York', '617,857', 'NA', 'US', 42.32, -71.14, 17920),
(2446, 'STANDARD', b'0', 'Brookline', NULL, NULL, 'MA', 'Norfolk County', 'America/New_York', '617,857', 'NA', 'US', 42.34, -71.12, 23060),
(2447, 'PO BOX', b'0', 'Brookline Village', 'Brookline Vlg', 'Brookline', 'MA', 'Norfolk County', 'America/New_York', '617', 'NA', 'US', 42.33, -71.13, 120),
(2451, 'STANDARD', b'0', 'Waltham', 'North Waltham', NULL, 'MA', 'Middlesex County', 'America/New_York', '339,617,781,508,978', 'NA', 'US', 42.38, -71.24, 16130),
(2452, 'STANDARD', b'0', 'Waltham', 'North Waltham', NULL, 'MA', 'Middlesex County', 'America/New_York', '508,978,339,617,781', 'NA', 'US', 42.39, -71.22, 10840),
(2453, 'STANDARD', b'0', 'Waltham', 'South Waltham', NULL, 'MA', 'Middlesex County', 'America/New_York', '508,978,339,617,781', 'NA', 'US', 42.37, -71.24, 23760),
(2454, 'PO BOX', b'0', 'Waltham', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', '339', 'NA', 'US', 42.38, -71.24, 543),
(2455, 'PO BOX', b'0', 'North Waltham', NULL, 'Waltham', 'MA', 'Middlesex County', 'America/New_York', '339', 'NA', 'US', 42.39, -71.22, 81),
(2456, 'PO BOX', b'0', 'New Town', NULL, 'Newton', 'MA', 'Middlesex County', 'America/New_York', '617', 'NA', 'US', 42.33, -71.20, 28),
(2457, 'PO BOX', b'0', 'Babson Park', NULL, 'Wellesley', 'MA', 'Norfolk County', 'America/New_York', '339', 'NA', 'US', 42.30, -71.27, 105),
(2458, 'STANDARD', b'0', 'Newton', 'Newtonville', 'Riverside', 'MA', 'Middlesex County', 'America/New_York', '617,857', 'NA', 'US', 42.35, -71.19, 11790),
(2459, 'STANDARD', b'0', 'Newton Center', 'Newton, Newton Centre', 'Newton Cntr, Newton Ctr', 'MA', 'Middlesex County', 'America/New_York', '617', 'NA', 'US', 42.31, -71.19, 17590),
(2460, 'STANDARD', b'0', 'Newtonville', 'Newton', NULL, 'MA', 'Middlesex County', 'America/New_York', '617,857', 'NA', 'US', 42.35, -71.20, 8820),
(2461, 'STANDARD', b'0', 'Newton Highlands', 'Newton, Newton Hlds', NULL, 'MA', 'Middlesex County', 'America/New_York', '617', 'NA', 'US', 42.32, -71.21, 7080),
(2462, 'STANDARD', b'0', 'Newton Lower Falls', 'Newton, Newton L F, Newtonville', NULL, 'MA', 'Middlesex County', 'America/New_York', '617', 'NA', 'US', 42.33, -71.26, 1310),
(2464, 'STANDARD', b'0', 'Newton Upper Falls', 'Newton, Newton U F', NULL, 'MA', 'Middlesex County', 'America/New_York', '617', 'NA', 'US', 42.31, -71.22, 2820),
(2465, 'STANDARD', b'0', 'West Newton', 'Newton', 'W Newton', 'MA', 'Middlesex County', 'America/New_York', '617,857', 'NA', 'US', 42.35, -71.22, 11730),
(2466, 'STANDARD', b'0', 'Auburndale', NULL, 'Newton', 'MA', 'Middlesex County', 'America/New_York', '617', 'NA', 'US', 42.34, -71.24, 6570),
(2467, 'STANDARD', b'0', 'Chestnut Hill', 'Boston Clg, Boston College', 'Brookline, Newton', 'MA', 'Norfolk County', 'America/New_York', '857,617', 'NA', 'US', 42.31, -71.16, 14600),
(2468, 'STANDARD', b'0', 'Waban', NULL, 'Newton', 'MA', 'Middlesex County', 'America/New_York', '617,781', 'NA', 'US', 42.32, -71.23, 5580),
(2471, 'PO BOX', b'0', 'Watertown', NULL, 'Watertown Financial', 'MA', 'Middlesex County', 'America/New_York', '617', 'NA', 'US', 42.36, -71.17, 273),
(2472, 'STANDARD', b'0', 'Watertown', 'E Watertown, East Watertown', NULL, 'MA', 'Middlesex County', 'America/New_York', '617,857', 'NA', 'US', 42.37, -71.18, 30280),
(2474, 'STANDARD', b'0', 'Arlington', 'E Arlington, East Arlington', NULL, 'MA', 'Middlesex County', 'America/New_York', '339,781', 'NA', 'US', 42.42, -71.16, 26100),
(2475, 'PO BOX', b'0', 'Arlington Heights', 'Arlington Hts', 'Arlington', 'MA', 'Middlesex County', 'America/New_York', '339', 'NA', 'US', 42.41, -71.18, 19),
(2476, 'STANDARD', b'0', 'Arlington', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', '339,781', 'NA', 'US', 42.41, -71.16, 16510),
(2477, 'UNIQUE', b'0', 'Watertown', NULL, 'Field Premium Inc', 'MA', 'Middlesex County', 'America/New_York', '617', 'NA', 'US', 42.36, -71.17, 0),
(2478, 'STANDARD', b'0', 'Belmont', NULL, NULL, 'MA', 'Middlesex County', 'America/New_York', '617,857', 'NA', 'US', 42.39, -71.18, 24720),
(2479, 'PO BOX', b'0', 'Waverley', NULL, 'Belmont', 'MA', 'Middlesex County', 'America/New_York', NULL, 'NA', 'US', 42.39, -71.17, 39),
(2481, 'STANDARD', b'0', 'Wellesley Hills', 'Wellesley, Wellesley Hls', 'Wellesley Fms', 'MA', 'Norfolk County', 'America/New_York', '508,774,339,781', 'NA', 'US', 42.31, -71.27, 15000),
(2482, 'STANDARD', b'0', 'Wellesley', NULL, NULL, 'MA', 'Norfolk County', 'America/New_York', '339,781,508,774', 'NA', 'US', 42.29, -71.30, 10250),
(2492, 'STANDARD', b'0', 'Needham', NULL, 'Needham Jct', 'MA', 'Norfolk County', 'America/New_York', '508,617,774,857,339,781', 'NA', 'US', 42.28, -71.24, 20330),
(2493, 'STANDARD', b'0', 'Weston', NULL, 'Cherry Brook, Hastings, Kendal Green, Silver Hill, Stony Brook', 'MA', 'Middlesex County', 'America/New_York', NULL, 'NA', 'US', 42.36, -71.30, 10830),
(2494, 'STANDARD', b'0', 'Needham Heights', 'Needham, Needham Hgts', NULL, 'MA', 'Norfolk County', 'America/New_York', '339,781,508,617,774,857', 'NA', 'US', 42.30, -71.23, 9810),
(2495, 'PO BOX', b'0', 'Nonantum', 'Newton', NULL, 'MA', 'Middlesex County', 'America/New_York', '617', 'NA', 'US', 42.33, -71.20, 30),
(2532, 'STANDARD', b'0', 'Buzzards Bay', 'Bourne', NULL, 'MA', 'Barnstable County', 'America/New_York', '508,774', 'NA', 'US', 41.75, -70.61, 9790),
(2534, 'PO BOX', b'0', 'Cataumet', NULL, NULL, 'MA', 'Barnstable County', 'America/New_York', '508', 'NA', 'US', 41.66, -70.61, 916),
(2535, 'STANDARD', b'0', 'Chilmark', 'Aquinnah, Gay Head', NULL, 'MA', 'Dukes County', 'America/New_York', '508', 'NA', 'US', 41.34, -70.74, 1030),
(2536, 'STANDARD', b'0', 'East Falmouth', 'E Falmouth, Ea Falmouth, Hatchville, Teaticket, Waquoit', NULL, 'MA', 'Barnstable County', 'America/New_York', '508', 'NA', 'US', 41.56, -70.55, 17840),
(2537, 'STANDARD', b'0', 'East Sandwich', 'E Sandwich', NULL, 'MA', 'Barnstable County', 'America/New_York', '508', 'NA', 'US', 41.73, -70.43, 5860),
(2538, 'STANDARD', b'0', 'East Wareham', 'E Wareham', NULL, 'MA', 'Plymouth County', 'America/New_York', '774', 'NA', 'US', 41.78, -70.65, 4110),
(2539, 'STANDARD', b'0', 'Edgartown', NULL, 'Chappaquiddick Island', 'MA', 'Dukes County', 'America/New_York', '508,774', 'NA', 'US', 41.38, -70.53, 4910),
(2540, 'STANDARD', b'0', 'Falmouth', NULL, NULL, 'MA', 'Barnstable County', 'America/New_York', '508,774', 'NA', 'US', 41.56, -70.62, 6320),
(2541, 'PO BOX', b'0', 'Falmouth', NULL, NULL, 'MA', 'Barnstable County', 'America/New_York', '508', 'NA', 'US', 41.54, -70.60, 594),
(2542, 'STANDARD', b'0', 'Buzzards Bay', 'Otis Angb', 'Otis A F B, Otis Air National Guard, Otis Ang', 'MA', 'Barnstable County', 'America/New_York', '508,774', 'NA', 'US', 41.71, -70.55, 560),
(2543, 'STANDARD', b'0', 'Woods Hole', 'Falmouth', 'Woodshole', 'MA', 'Barnstable County', 'America/New_York', '508,774', 'NA', 'US', 41.52, -70.66, 700),
(2552, 'PO BOX', b'0', 'Menemsha', NULL, 'Chilmark', 'MA', 'Dukes County', 'America/New_York', NULL, 'NA', 'US', 41.34, -70.73, 46),
(2553, 'PO BOX', b'0', 'Monument Beach', 'Monument Bch', NULL, 'MA', 'Barnstable County', 'America/New_York', '508', 'NA', 'US', 41.71, -70.62, 1171),
(2554, 'STANDARD', b'0', 'Nantucket', NULL, NULL, 'MA', 'Nantucket County', 'America/New_York', '508,774', 'NA', 'US', 41.27, -70.10, 9870),
(2556, 'STANDARD', b'0', 'North Falmouth', 'N Falmouth', NULL, 'MA', 'Barnstable County', 'America/New_York', '508,774', 'NA', 'US', 41.64, -70.63, 3210),
(2557, 'STANDARD', b'0', 'Oak Bluffs', NULL, NULL, 'MA', 'Dukes County', 'America/New_York', NULL, 'NA', 'US', 41.45, -70.56, 2660),
(2558, 'PO BOX', b'0', 'Onset', NULL, NULL, 'MA', 'Plymouth County', 'America/New_York', '508', 'NA', 'US', 41.74, -70.66, 2189),
(2559, 'STANDARD', b'0', 'Pocasset', NULL, 'Bourne', 'MA', 'Barnstable County', 'America/New_York', '508,774', 'NA', 'US', 41.69, -70.63, 2920),
(2561, 'PO BOX', b'0', 'Sagamore', NULL, NULL, 'MA', 'Barnstable County', 'America/New_York', '508', 'NA', 'US', 41.77, -70.53, 884),
(2562, 'STANDARD', b'0', 'Sagamore Beach', 'Sagamore Bch', NULL, 'MA', 'Barnstable County', 'America/New_York', '508,774', 'NA', 'US', 41.79, -70.53, 3050),
(2563, 'STANDARD', b'0', 'Sandwich', NULL, NULL, 'MA', 'Barnstable County', 'America/New_York', '774,508', 'NA', 'US', 41.75, -70.49, 9860),
(2564, 'PO BOX', b'0', 'Siasconset', 'Nantucket', 'Sconset', 'MA', 'Nantucket County', 'America/New_York', '508', 'NA', 'US', 41.27, -70.00, 423),
(2565, 'PO BOX', b'0', 'Silver Beach', 'N Falmouth, North Falmouth', NULL, 'MA', 'Barnstable County', 'America/New_York', '508', 'NA', 'US', 41.63, -70.64, 0),
(2568, 'STANDARD', b'0', 'Vineyard Haven', 'Vineyard Hvn', 'North Tisbury, Tisbury, West Tisbury', 'MA', 'Dukes County', 'America/New_York', '508,774', 'NA', 'US', 41.45, -70.60, 6830),
(2571, 'STANDARD', b'0', 'Wareham', NULL, NULL, 'MA', 'Plymouth County', 'America/New_York', '508,774', 'NA', 'US', 41.76, -70.71, 9080),
(2573, 'PO BOX', b'0', 'West Chop', 'Vineyard Haven, Vineyard Hvn', 'Tisbury', 'MA', 'Dukes County', 'America/New_York', NULL, 'NA', 'US', 41.45, -70.60, 0),
(2574, 'PO BOX', b'0', 'West Falmouth', 'W Falmouth', NULL, 'MA', 'Barnstable County', 'America/New_York', '508', 'NA', 'US', 41.60, -70.64, 1104),
(2575, 'PO BOX', b'0', 'West Tisbury', NULL, 'Tisbury', 'MA', 'Dukes County', 'America/New_York', NULL, 'NA', 'US', 41.38, -70.67, 1679),
(2576, 'STANDARD', b'0', 'West Wareham', NULL, 'W Wareham', 'MA', 'Plymouth County', 'America/New_York', '508,774', 'NA', 'US', 41.78, -70.75, 3720),
(2584, 'PO BOX', b'0', 'Nantucket', NULL, NULL, 'MA', 'Nantucket County', 'America/New_York', '508', 'NA', 'US', 41.26, -70.01, 1904),
(2601, 'STANDARD', b'0', 'Hyannis', NULL, NULL, 'MA', 'Barnstable County', 'America/New_York', '508,774', 'NA', 'US', 41.65, -70.29, 12890),
(2630, 'STANDARD', b'0', 'Barnstable', NULL, NULL, 'MA', 'Barnstable County', 'America/New_York', '508,774', 'NA', 'US', 41.70, -70.30, 2000),
(2631, 'STANDARD', b'0', 'Brewster', NULL, NULL, 'MA', 'Barnstable County', 'America/New_York', '508,774', 'NA', 'US', 41.76, -70.08, 8660),
(2632, 'STANDARD', b'0', 'Centerville', NULL, NULL, 'MA', 'Barnstable County', 'America/New_York', '508,774', 'NA', 'US', 41.66, -70.34, 9890),
(2633, 'STANDARD', b'0', 'Chatham', NULL, NULL, 'MA', 'Barnstable County', 'America/New_York', '508,774', 'NA', 'US', 41.67, -69.96, 3550),
(2634, 'PO BOX', b'0', 'Centerville', NULL, 'Barnstable', 'MA', 'Barnstable County', 'America/New_York', '508', 'NA', 'US', 41.66, -70.34, 19),
(2635, 'STANDARD', b'0', 'Cotuit', NULL, NULL, 'MA', 'Barnstable County', 'America/New_York', '508', 'NA', 'US', 41.62, -70.44, 3340),
(2636, 'STANDARD', b'1', 'Centerville', NULL, NULL, 'MA', 'Barnstable County', 'America/New_York', '508', 'NA', 'US', 41.64, -70.34, 0),
(2637, 'PO BOX', b'0', 'Cummaquid', NULL, NULL, 'MA', 'Barnstable County', 'America/New_York', '508', 'NA', 'US', 41.71, -70.27, 564),
(2638, 'STANDARD', b'0', 'Dennis', NULL, NULL, 'MA', 'Barnstable County', 'America/New_York', '508,774', 'NA', 'US', 41.73, -70.20, 2720),
(2639, 'STANDARD', b'0', 'Dennis Port', 'Dennisport', NULL, 'MA', 'Barnstable County', 'America/New_York', '508', 'NA', 'US', 41.66, -70.13, 2740),
(2641, 'PO BOX', b'0', 'East Dennis', NULL, 'E Dennis', 'MA', 'Barnstable County', 'America/New_York', '508', 'NA', 'US', 41.75, -70.15, 1542),
(2642, 'STANDARD', b'0', 'Eastham', NULL, NULL, 'MA', 'Barnstable County', 'America/New_York', '508', 'NA', 'US', 41.83, -69.96, 3470),
(2643, 'PO BOX', b'0', 'East Orleans', NULL, NULL, 'MA', 'Barnstable County', 'America/New_York', '508', 'NA', 'US', 41.80, -69.94, 1113),
(2644, 'STANDARD', b'0', 'Forestdale', NULL, 'Sandwich', 'MA', 'Barnstable County', 'America/New_York', '508', 'NA', 'US', 41.68, -70.50, 4150),
(2645, 'STANDARD', b'0', 'Harwich', 'E Harwich, East Harwich', 'Hardwich', 'MA', 'Barnstable County', 'America/New_York', '508,774', 'NA', 'US', 41.69, -70.07, 9240),
(2646, 'STANDARD', b'0', 'Harwich Port', NULL, 'Harwichport', 'MA', 'Barnstable County', 'America/New_York', '508', 'NA', 'US', 41.67, -70.07, 1820),
(2647, 'PO BOX', b'0', 'Hyannis Port', NULL, NULL, 'MA', 'Barnstable County', 'America/New_York', '508', 'NA', 'US', 41.63, -70.31, 393),
(2648, 'STANDARD', b'0', 'Marstons Mills', 'Marstons Mls', NULL, 'MA', 'Barnstable County', 'America/New_York', '508', 'NA', 'US', 41.67, -70.40, 7110),
(2649, 'STANDARD', b'0', 'Mashpee', NULL, 'New Seabury, South Mashpee', 'MA', 'Barnstable County', 'America/New_York', '508', 'NA', 'US', 41.65, -70.48, 13490),
(2650, 'STANDARD', b'0', 'North Chatham', NULL, 'N Chatham', 'MA', 'Barnstable County', 'America/New_York', '508', 'NA', 'US', 41.70, -69.95, 790),
(2651, 'PO BOX', b'0', 'North Eastham', NULL, NULL, 'MA', 'Barnstable County', 'America/New_York', '508', 'NA', 'US', 41.87, -70.00, 1610),
(2652, 'PO BOX', b'0', 'North Truro', NULL, NULL, 'MA', 'Barnstable County', 'America/New_York', '508', 'NA', 'US', 42.04, -70.09, 766),
(2653, 'STANDARD', b'0', 'Orleans', NULL, NULL, 'MA', 'Barnstable County', 'America/New_York', '508,774', 'NA', 'US', 41.79, -70.00, 4640),
(2655, 'STANDARD', b'0', 'Osterville', NULL, NULL, 'MA', 'Barnstable County', 'America/New_York', '508,774', 'NA', 'US', 41.62, -70.38, 3320),
(2657, 'STANDARD', b'0', 'Provincetown', NULL, NULL, 'MA', 'Barnstable County', 'America/New_York', '508,774', 'NA', 'US', 42.06, -70.20, 3410),
(2659, 'STANDARD', b'0', 'South Chatham', NULL, NULL, 'MA', 'Barnstable County', 'America/New_York', '508', 'NA', 'US', 41.68, -70.02, 1060),
(2660, 'STANDARD', b'0', 'South Dennis', NULL, 'S Dennis', 'MA', 'Barnstable County', 'America/New_York', '508', 'NA', 'US', 41.70, -70.15, 5380),
(2661, 'PO BOX', b'0', 'South Harwich', NULL, NULL, 'MA', 'Barnstable County', 'America/New_York', '508', 'NA', 'US', 41.67, -70.04, 365),
(2662, 'PO BOX', b'0', 'South Orleans', NULL, NULL, 'MA', 'Barnstable County', 'America/New_York', '508', 'NA', 'US', 41.75, -69.99, 862),
(2663, 'PO BOX', b'0', 'South Wellfleet', 'S Wellfleet', NULL, 'MA', 'Barnstable County', 'America/New_York', '508', 'NA', 'US', 41.89, -70.01, 611),
(2664, 'STANDARD', b'0', 'South Yarmouth', 'Bass River, S Yarmouth', 'So Yarmouth, Yarmouth', 'MA', 'Barnstable County', 'America/New_York', '508', 'NA', 'US', 41.67, -70.20, 8460),
(2666, 'PO BOX', b'0', 'Truro', NULL, NULL, 'MA', 'Barnstable County', 'America/New_York', '508', 'NA', 'US', 42.00, -70.06, 1057),
(2667, 'STANDARD', b'0', 'Wellfleet', NULL, NULL, 'MA', 'Barnstable County', 'America/New_York', '508,774', 'NA', 'US', 41.93, -70.03, 2340),
(2668, 'STANDARD', b'0', 'West Barnstable', 'W Barnstble', 'W Barnstable', 'MA', 'Barnstable County', 'America/New_York', '508', 'NA', 'US', 41.70, -70.37, 3140),
(2669, 'PO BOX', b'0', 'West Chatham', NULL, NULL, 'MA', 'Barnstable County', 'America/New_York', '508', 'NA', 'US', 41.68, -69.99, 812),
(2670, 'STANDARD', b'0', 'West Dennis', NULL, 'W Dennis', 'MA', 'Barnstable County', 'America/New_York', '508', 'NA', 'US', 41.66, -70.16, 1280),
(2671, 'STANDARD', b'0', 'West Harwich', NULL, NULL, 'MA', 'Barnstable County', 'America/New_York', '508', 'NA', 'US', 41.67, -70.11, 1040),
(2672, 'PO BOX', b'0', 'West Hyannisport', 'W Hyannisprt', NULL, 'MA', 'Barnstable County', 'America/New_York', '508', 'NA', 'US', 41.64, -70.31, 577),
(2673, 'STANDARD', b'0', 'West Yarmouth', 'W Yarmouth', 'South Yarmouth, Yarmouth', 'MA', 'Barnstable County', 'America/New_York', '508,774', 'NA', 'US', 41.65, -70.24, 7530),
(2675, 'STANDARD', b'0', 'Yarmouth Port', NULL, 'Yarmouth, Yarmouthport', 'MA', 'Barnstable County', 'America/New_York', '508', 'NA', 'US', 41.70, -70.22, 6030),
(2702, 'STANDARD', b'0', 'Assonet', NULL, 'Freetown', 'MA', 'Bristol County', 'America/New_York', '508,774', 'NA', 'US', 41.78, -71.06, 4050),
(2703, 'STANDARD', b'0', 'Attleboro', 'S Attleboro, South Attleboro', NULL, 'MA', 'Bristol County', 'America/New_York', '508,774', 'NA', 'US', 41.93, -71.29, 40590),
(2712, 'PO BOX', b'0', 'Chartley', NULL, 'Norton', 'MA', 'Bristol County', 'America/New_York', '508', 'NA', 'US', 41.97, -71.18, 272),
(2713, 'PO BOX', b'0', 'Cuttyhunk', NULL, 'Gosnold', 'MA', 'Dukes County', 'America/New_York', NULL, 'NA', 'US', 41.44, -70.90, 32),
(2714, 'PO BOX', b'0', 'Dartmouth', NULL, NULL, 'MA', 'Bristol County', 'America/New_York', '508', 'NA', 'US', 41.56, -71.00, 47),
(2715, 'STANDARD', b'0', 'Dighton', NULL, NULL, 'MA', 'Bristol County', 'America/New_York', '508,774', 'NA', 'US', 41.82, -71.16, 3290),
(2717, 'STANDARD', b'0', 'East Freetown', NULL, NULL, 'MA', 'Bristol County', 'America/New_York', '508', 'NA', 'US', 41.75, -70.97, 4710),
(2718, 'STANDARD', b'0', 'East Taunton', NULL, 'Taunton', 'MA', 'Bristol County', 'America/New_York', '508,774', 'NA', 'US', 41.87, -71.01, 6500),
(2719, 'STANDARD', b'0', 'Fairhaven', NULL, NULL, 'MA', 'Bristol County', 'America/New_York', '508', 'NA', 'US', 41.63, -70.90, 13840),
(2720, 'STANDARD', b'0', 'Fall River', NULL, NULL, 'MA', 'Bristol County', 'America/New_York', '508,774', 'NA', 'US', 41.73, -71.12, 24080),
(2721, 'STANDARD', b'0', 'Fall River', NULL, NULL, 'MA', 'Bristol County', 'America/New_York', '508,774', 'NA', 'US', 41.68, -71.15, 20150),
(2722, 'PO BOX', b'0', 'Fall River', NULL, NULL, 'MA', 'Bristol County', 'America/New_York', '508', 'NA', 'US', 41.71, -71.10, 738),
(2723, 'STANDARD', b'0', 'Fall River', NULL, NULL, 'MA', 'Bristol County', 'America/New_York', '508,774', 'NA', 'US', 41.69, -71.13, 11530),
(2724, 'STANDARD', b'0', 'Fall River', NULL, NULL, 'MA', 'Bristol County', 'America/New_York', '508', 'NA', 'US', 41.68, -71.18, 13000),
(2725, 'STANDARD', b'0', 'Somerset', NULL, 'Fall River', 'MA', 'Bristol County', 'America/New_York', '508', 'NA', 'US', 41.72, -71.19, 2330),
(2726, 'STANDARD', b'0', 'Somerset', NULL, 'Fall River', 'MA', 'Bristol County', 'America/New_York', '508', 'NA', 'US', 41.73, -71.15, 14380),
(2738, 'STANDARD', b'0', 'Marion', NULL, NULL, 'MA', 'Plymouth County', 'America/New_York', '508,774', 'NA', 'US', 41.70, -70.76, 4950),
(2739, 'STANDARD', b'0', 'Mattapoisett', NULL, NULL, 'MA', 'Plymouth County', 'America/New_York', '508,774', 'NA', 'US', 41.66, -70.80, 6220),
(2740, 'STANDARD', b'0', 'New Bedford', NULL, NULL, 'MA', 'Bristol County', 'America/New_York', '508,774', 'NA', 'US', 41.64, -70.94, 33170),
(2741, 'PO BOX', b'0', 'New Bedford', NULL, NULL, 'MA', 'Bristol County', 'America/New_York', '508', 'NA', 'US', 41.66, -70.93, 157),
(2742, 'PO BOX', b'0', 'New Bedford', NULL, NULL, 'MA', 'Bristol County', 'America/New_York', '508', 'NA', 'US', 41.66, -70.93, 334),
(2743, 'STANDARD', b'0', 'Acushnet', 'New Bedford', NULL, 'MA', 'Bristol County', 'America/New_York', '508', 'NA', 'US', 41.68, -70.90, 9580),
(2744, 'STANDARD', b'0', 'New Bedford', NULL, NULL, 'MA', 'Bristol County', 'America/New_York', '508,774', 'NA', 'US', 41.61, -70.91, 9450),
(2745, 'STANDARD', b'0', 'New Bedford', 'Acushnet', NULL, 'MA', 'Bristol County', 'America/New_York', '508', 'NA', 'US', 41.70, -70.95, 21310),
(2746, 'STANDARD', b'0', 'New Bedford', NULL, NULL, 'MA', 'Bristol County', 'America/New_York', '508,774', 'NA', 'US', 41.66, -70.93, 11960),
(2747, 'STANDARD', b'0', 'North Dartmouth', 'Dartmouth, N Dartmouth', NULL, 'MA', 'Bristol County', 'America/New_York', '508', 'NA', 'US', 41.64, -71.00, 16910),
(2748, 'STANDARD', b'0', 'South Dartmouth', 'Dartmouth, Nonquitt, S Dartmouth', 'Padanaram The Packet', 'MA', 'Bristol County', 'America/New_York', '508', 'NA', 'US', 41.55, -70.98, 10470),
(2760, 'STANDARD', b'0', 'North Attleboro', 'N Attleboro', 'No Attleboro', 'MA', 'Bristol County', 'America/New_York', '508,774', 'NA', 'US', 41.97, -71.33, 26000),
(2761, 'PO BOX', b'0', 'North Attleboro', 'N Attleboro', NULL, 'MA', 'Bristol County', 'America/New_York', '508', 'NA', 'US', 41.97, -71.33, 356),
(2762, 'STANDARD', b'0', 'Plainville', NULL, 'N Attleboro, North Attleboro', 'MA', 'Norfolk County', 'America/New_York', NULL, 'NA', 'US', 42.00, -71.33, 8470),
(2763, 'STANDARD', b'0', 'Attleboro Falls', 'Attleboro Fls, N Attleboro, North Attleboro', NULL, 'MA', 'Bristol County', 'America/New_York', '508,774', 'NA', 'US', 41.97, -71.31, 2030),
(2764, 'STANDARD', b'0', 'North Dighton', 'N Dighton', NULL, 'MA', 'Bristol County', 'America/New_York', '508', 'NA', 'US', 41.85, -71.15, 3980),
(2766, 'STANDARD', b'0', 'Norton', NULL, NULL, 'MA', 'Bristol County', 'America/New_York', '508,774', 'NA', 'US', 41.96, -71.18, 16530),
(2767, 'STANDARD', b'0', 'Raynham', NULL, NULL, 'MA', 'Bristol County', 'America/New_York', '508', 'NA', 'US', 41.93, -71.04, 13220),
(2768, 'PO BOX', b'0', 'Raynham Center', 'Raynham Ctr', 'Raynham', 'MA', 'Bristol County', 'America/New_York', '508', 'NA', 'US', 41.93, -71.04, 502),
(2769, 'STANDARD', b'0', 'Rehoboth', NULL, NULL, 'MA', 'Bristol County', 'America/New_York', '508,774', 'NA', 'US', 41.83, -71.26, 11520),
(2770, 'STANDARD', b'0', 'Rochester', NULL, NULL, 'MA', 'Plymouth County', 'America/New_York', '508,774', 'NA', 'US', 41.73, -70.81, 5320),
(2771, 'STANDARD', b'0', 'Seekonk', NULL, NULL, 'MA', 'Bristol County', 'America/New_York', '508,774', 'NA', 'US', 41.81, -71.33, 13890),
(2777, 'STANDARD', b'0', 'Swansea', NULL, NULL, 'MA', 'Bristol County', 'America/New_York', '508', 'NA', 'US', 41.75, -71.18, 15040),
(2779, 'STANDARD', b'0', 'Berkley', NULL, NULL, 'MA', 'Bristol County', 'America/New_York', '508', 'NA', 'US', 41.85, -71.08, 6360),
(2780, 'STANDARD', b'0', 'Taunton', NULL, NULL, 'MA', 'Bristol County', 'America/New_York', '508,774', 'NA', 'US', 41.90, -71.09, 43620),
(2783, 'UNIQUE', b'0', 'Taunton', NULL, 'Chadwicks', 'MA', 'Bristol County', 'America/New_York', '508', 'NA', 'US', 41.90, -71.09, 0),
(2790, 'STANDARD', b'0', 'Westport', NULL, 'Horseneck Beach', 'MA', 'Bristol County', 'America/New_York', '508,774', 'NA', 'US', 41.66, -71.10, 14990),
(2791, 'PO BOX', b'0', 'Westport Point', 'Westport Pt', NULL, 'MA', 'Bristol County', 'America/New_York', '508', 'NA', 'US', 41.52, -71.07, 388),
(2801, 'PO BOX', b'0', 'Adamsville', NULL, 'Little Compton', 'RI', 'Newport County', 'America/New_York', '401', 'NA', 'US', 41.51, -71.16, 299),
(2802, 'PO BOX', b'0', 'Albion', NULL, 'Lincoln', 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.95, -71.46, 770),
(2804, 'STANDARD', b'0', 'Ashaway', NULL, 'Hopkinton', 'RI', 'Washington County', 'America/New_York', '401', 'NA', 'US', 41.42, -71.78, 2640),
(2806, 'STANDARD', b'0', 'Barrington', NULL, NULL, 'RI', 'Bristol County', 'America/New_York', '401', 'NA', 'US', 41.73, -71.31, 16810),
(2807, 'PO BOX', b'0', 'Block Island', 'New Shoreham', NULL, 'RI', 'Washington County', 'America/New_York', '401', 'NA', 'US', 41.16, -71.58, 1213),
(2808, 'STANDARD', b'0', 'Bradford', NULL, 'Hopkinton, Westerly', 'RI', 'Washington County', 'America/New_York', '401', 'NA', 'US', 41.39, -71.75, 2140),
(2809, 'STANDARD', b'0', 'Bristol', NULL, NULL, 'RI', 'Bristol County', 'America/New_York', '401', 'NA', 'US', 41.67, -71.27, 17300),
(2812, 'STANDARD', b'0', 'Carolina', 'Richmond', NULL, 'RI', 'Washington County', 'America/New_York', '401', 'NA', 'US', 41.47, -71.64, 1310),
(2813, 'STANDARD', b'0', 'Charlestown', NULL, NULL, 'RI', 'Washington County', 'America/New_York', '401', 'NA', 'US', 41.38, -71.65, 6920),
(2814, 'STANDARD', b'0', 'Chepachet', NULL, 'Glocester', 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.91, -71.70, 6900),
(2815, 'STANDARD', b'0', 'Clayville', NULL, 'Scituate', 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.77, -71.66, 240),
(2816, 'STANDARD', b'0', 'Coventry', NULL, NULL, 'RI', 'Kent County', 'America/New_York', '401', 'NA', 'US', 41.68, -71.66, 29700),
(2817, 'STANDARD', b'0', 'West Greenwich', 'W Greenwich', NULL, 'RI', 'Kent County', 'America/New_York', '401', 'NA', 'US', 41.63, -71.65, 5860),
(2818, 'STANDARD', b'0', 'East Greenwich', 'E Greenwich', 'Warwick', 'RI', 'Kent County', 'America/New_York', '401', 'NA', 'US', 41.63, -71.50, 18030),
(2822, 'STANDARD', b'0', 'Exeter', 'Escoheag', NULL, 'RI', 'Washington County', 'America/New_York', '401', 'NA', 'US', 41.56, -71.68, 5550),
(2823, 'PO BOX', b'0', 'Fiskeville', NULL, 'Cranston', 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.73, -71.54, 302),
(2824, 'PO BOX', b'0', 'Forestdale', NULL, NULL, 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.97, -71.55, 394),
(2825, 'STANDARD', b'0', 'Foster', NULL, 'Scituate', 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.85, -71.76, 4960),
(2826, 'PO BOX', b'0', 'Glendale', NULL, 'Burrillville', 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.98, -71.65, 880),
(2827, 'STANDARD', b'0', 'Greene', 'Coventry', NULL, 'RI', 'Kent County', 'America/New_York', '401', 'NA', 'US', 41.69, -71.74, 2130),
(2828, 'STANDARD', b'0', 'Greenville', NULL, 'Smithfield', 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.87, -71.55, 6660),
(2829, 'PO BOX', b'0', 'Harmony', NULL, 'Glocester', 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.88, -71.61, 405),
(2830, 'STANDARD', b'0', 'Harrisville', 'Burrillville', NULL, 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.96, -71.67, 5940),
(2831, 'STANDARD', b'0', 'Hope', NULL, 'Scituate', 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.75, -71.56, 3860),
(2832, 'STANDARD', b'0', 'Hope Valley', 'Richmond', 'Hopkinton', 'RI', 'Washington County', 'America/New_York', '401', 'NA', 'US', 41.51, -71.72, 4410),
(2833, 'STANDARD', b'0', 'Hopkinton', NULL, NULL, 'RI', 'Washington County', 'America/New_York', '401', 'NA', 'US', 41.48, -71.77, 550),
(2835, 'STANDARD', b'0', 'Jamestown', NULL, NULL, 'RI', 'Newport County', 'America/New_York', '401', 'NA', 'US', 41.48, -71.36, 5230),
(2836, 'STANDARD', b'0', 'Kenyon', 'Richmond', 'Charlestown', 'RI', 'Washington County', 'America/New_York', '401', 'NA', 'US', 41.45, -71.62, 91),
(2837, 'STANDARD', b'0', 'Little Compton', 'L Compton', 'Adamsville', 'RI', 'Newport County', 'America/New_York', '401', 'NA', 'US', 41.50, -71.16, 3060),
(2838, 'STANDARD', b'0', 'Manville', NULL, 'Lincoln', 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.96, -71.47, 3020),
(2839, 'STANDARD', b'0', 'Mapleville', NULL, 'Burrillville', 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.94, -71.64, 1620),
(2840, 'STANDARD', b'0', 'Newport', NULL, 'Middletown', 'RI', 'Newport County', 'America/New_York', '401', 'NA', 'US', 41.47, -71.30, 18320),
(2841, 'STANDARD', b'0', 'Newport', NULL, 'Netc', 'RI', 'Newport County', 'America/New_York', '401', 'NA', 'US', 41.51, -71.33, 353),
(2842, 'STANDARD', b'0', 'Middletown', NULL, NULL, 'RI', 'Newport County', 'America/New_York', '401', 'NA', 'US', 41.51, -71.27, 15150),
(2852, 'STANDARD', b'0', 'North Kingstown', 'N Kingstown', 'Davisville, Wickford', 'RI', 'Washington County', 'America/New_York', '401', 'NA', 'US', 41.55, -71.46, 21530),
(2854, 'STANDARD', b'1', 'North Kingstown', 'N Kingstown', NULL, 'RI', 'Washington County', 'America/New_York', '401', 'NA', 'US', 41.59, -71.45, 0),
(2857, 'STANDARD', b'0', 'North Scituate', 'N Scituate, Scituate', 'Glocester', 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.83, -71.63, 8010),
(2858, 'STANDARD', b'0', 'Oakland', NULL, 'Burrillville', 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.97, -71.65, 520),
(2859, 'STANDARD', b'0', 'Pascoag', NULL, 'Glocester', 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.95, -71.70, 5610),
(2860, 'STANDARD', b'0', 'Pawtucket', NULL, NULL, 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.87, -71.37, 38130),
(2861, 'STANDARD', b'0', 'Pawtucket', NULL, 'Darlington', 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.88, -71.35, 23950),
(2862, 'PO BOX', b'0', 'Pawtucket', NULL, NULL, 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.87, -71.37, 912),
(2863, 'STANDARD', b'0', 'Central Falls', NULL, NULL, 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.89, -71.39, 15900),
(2864, 'STANDARD', b'0', 'Cumberland', NULL, NULL, 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.94, -71.41, 32390),
(2865, 'STANDARD', b'0', 'Lincoln', NULL, NULL, 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.91, -71.45, 16580),
(2871, 'STANDARD', b'0', 'Portsmouth', NULL, NULL, 'RI', 'Newport County', 'America/New_York', '401', 'NA', 'US', 41.60, -71.25, 16410),
(2872, 'PO BOX', b'0', 'Prudence Island', 'Prudence Isl', 'Portsmouth', 'RI', 'Newport County', 'America/New_York', '401', 'NA', 'US', 41.60, -71.31, 160),
(2873, 'PO BOX', b'0', 'Rockville', NULL, 'Hopkinton', 'RI', 'Washington County', 'America/New_York', '401', 'NA', 'US', 41.53, -71.78, 301),
(2874, 'STANDARD', b'0', 'Saunderstown', NULL, 'Narragansett, North Kingstown', 'RI', 'Washington County', 'America/New_York', '401', 'NA', 'US', 41.51, -71.44, 5810),
(2875, 'STANDARD', b'0', 'Shannock', 'Richmond', NULL, 'RI', 'Washington County', 'America/New_York', '401', 'NA', 'US', 41.46, -71.64, 350),
(2876, 'STANDARD', b'0', 'Slatersville', NULL, NULL, 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.99, -71.59, 1180),
(2877, 'STANDARD', b'0', 'Slocum', NULL, NULL, 'RI', 'Washington County', 'America/New_York', '401', 'NA', 'US', 41.52, -71.54, 106),
(2878, 'STANDARD', b'0', 'Tiverton', NULL, NULL, 'RI', 'Newport County', 'America/New_York', '401', 'NA', 'US', 41.65, -71.20, 14500),
(2879, 'STANDARD', b'0', 'Wakefield', 'Narragansett, Peace Dale, S Kingstown, South Kingstown', 'East Matunuck, Green Hill, Jerusalem, Matunuck', 'RI', 'Washington County', 'America/New_York', '401', 'NA', 'US', 41.45, -71.51, 18750),
(2880, 'PO BOX', b'0', 'Wakefield', NULL, NULL, 'RI', 'Washington County', 'America/New_York', '401', 'NA', 'US', 41.45, -71.51, 684),
(2881, 'STANDARD', b'0', 'Kingston', NULL, 'Wakefield', 'RI', 'Washington County', 'America/New_York', '401', 'NA', 'US', 41.48, -71.52, 1940),
(2882, 'STANDARD', b'0', 'Narragansett', 'Point Judith', 'Bonnet Shores, Galilee', 'RI', 'Washington County', 'America/New_York', '401', 'NA', 'US', 41.39, -71.48, 10070),
(2883, 'PO BOX', b'0', 'Peace Dale', 'S Kingstown, South Kingstown', 'Wakefield', 'RI', 'Washington County', 'America/New_York', '401', 'NA', 'US', 41.45, -71.50, 217),
(2885, 'STANDARD', b'0', 'Warren', NULL, NULL, 'RI', 'Bristol County', 'America/New_York', '401', 'NA', 'US', 41.72, -71.26, 9180),
(2886, 'STANDARD', b'0', 'Warwick', NULL, NULL, 'RI', 'Kent County', 'America/New_York', '401', 'NA', 'US', 41.70, -71.46, 26080),
(2887, 'PO BOX', b'0', 'Warwick', NULL, NULL, 'RI', 'Kent County', 'America/New_York', '401', 'NA', 'US', 41.70, -71.42, 344),
(2888, 'STANDARD', b'0', 'Warwick', NULL, NULL, 'RI', 'Kent County', 'America/New_York', '401', 'NA', 'US', 41.75, -71.41, 18150),
(2889, 'STANDARD', b'0', 'Warwick', NULL, 'Conimicut', 'RI', 'Kent County', 'America/New_York', '401', 'NA', 'US', 41.70, -71.42, 25560),
(2891, 'STANDARD', b'0', 'Westerly', NULL, 'Misquamicut, Watch Hill', 'RI', 'Washington County', 'America/New_York', '401', 'NA', 'US', 41.37, -71.81, 19460),
(2892, 'STANDARD', b'0', 'West Kingston', 'Richmond', 'South Kingstown', 'RI', 'Washington County', 'America/New_York', '401', 'NA', 'US', 41.50, -71.59, 5000),
(2893, 'STANDARD', b'0', 'West Warwick', NULL, 'W Warwick', 'RI', 'Kent County', 'America/New_York', '401', 'NA', 'US', 41.69, -71.51, 25880),
(2894, 'STANDARD', b'0', 'Wood River Junction', 'Wood River Jt', NULL, 'RI', 'Washington County', 'America/New_York', '401', 'NA', 'US', 41.45, -71.70, 760),
(2895, 'STANDARD', b'0', 'Woonsocket', NULL, NULL, 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.99, -71.50, 33800),
(2896, 'STANDARD', b'0', 'North Smithfield', 'N Smithfield', NULL, 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.95, -71.55, 9360),
(2898, 'STANDARD', b'0', 'Wyoming', 'Richmond', NULL, 'RI', 'Washington County', 'America/New_York', '401', 'NA', 'US', 41.52, -71.67, 1860),
(2901, 'PO BOX', b'0', 'Providence', NULL, NULL, 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.82, -71.41, 299),
(2902, 'UNIQUE', b'0', 'Providence', NULL, 'Providence Journal', 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.82, -71.41, 69),
(2903, 'STANDARD', b'0', 'Providence', NULL, NULL, 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.82, -71.41, 5350),
(2904, 'STANDARD', b'0', 'Providence', 'N Providence, North Providence', 'No Providence', 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.86, -71.44, 24910),
(2905, 'STANDARD', b'0', 'Providence', 'Cranston', NULL, 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.78, -71.40, 21040),
(2906, 'STANDARD', b'0', 'Providence', NULL, NULL, 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.84, -71.39, 19250),
(2907, 'STANDARD', b'0', 'Providence', 'Cranston', NULL, 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.80, -71.42, 25280),
(2908, 'STANDARD', b'0', 'Providence', 'N Providence, North Providence', NULL, 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.84, -71.44, 29520),
(2909, 'STANDARD', b'0', 'Providence', NULL, NULL, 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.82, -71.45, 35900),
(2910, 'STANDARD', b'0', 'Cranston', 'Providence', NULL, 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.77, -71.44, 20320),
(2911, 'STANDARD', b'0', 'North Providence', 'N Providence, Providence', 'Centerdale, Centredale, Centredale Finance Branch, No Providence', 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.85, -71.47, 13450),
(2912, 'UNIQUE', b'0', 'Providence', 'Brown Station', 'Brown University', 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.83, -71.40, 375),
(2914, 'STANDARD', b'0', 'East Providence', 'E Providence', NULL, 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.81, -71.37, 18220),
(2915, 'STANDARD', b'0', 'Riverside', NULL, 'East Providence', 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.77, -71.35, 14630),
(2916, 'STANDARD', b'0', 'Rumford', NULL, 'East Providence', 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.84, -71.35, 7730),
(2917, 'STANDARD', b'0', 'Smithfield', NULL, 'Esmond', 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.90, -71.53, 10610),
(2918, 'UNIQUE', b'0', 'Providence', 'Friar Station', 'Providence College', 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.82, -71.41, 36),
(2919, 'STANDARD', b'0', 'Johnston', 'Providence', NULL, 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.83, -71.52, 25900),
(2920, 'STANDARD', b'0', 'Cranston', NULL, NULL, 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.77, -71.47, 31070),
(2921, 'STANDARD', b'0', 'Cranston', NULL, NULL, 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.76, -71.48, 11690),
(2940, 'PO BOX', b'0', 'Providence', NULL, NULL, 'RI', 'Providence County', 'America/New_York', '401', 'NA', 'US', 41.82, -71.41, 1143),
(100, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'AL', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(101, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'AK', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(102, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'AZ', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(103, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'AR', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(104, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'CA', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(105, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'CO', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(106, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'CT', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(107, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'DE', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(108, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'FL', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(109, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'GA', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(110, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'HI', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(111, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'ID', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(112, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'IL', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(113, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'IN', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(114, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'IA', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(115, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'KS', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(116, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'KY', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(117, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'LA', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(118, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'ME', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(119, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'MD', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(120, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'MA', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(121, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'MI', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(122, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'MN', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(123, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'MS', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(124, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'MO', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(125, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'MT', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(126, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'NE', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(127, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'NV', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(128, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'NH', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(129, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'NJ', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(130, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'NM', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(131, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'NY', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(132, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'NC', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(133, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'ND', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(134, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'OH', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(135, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'OK', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(136, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'OR', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(137, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'PA', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(138, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'RI', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(139, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'SC', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(140, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'SD', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(141, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'TN', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(142, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'TX', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(143, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'UT', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(144, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'VT', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(145, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'VA', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(146, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'WA', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(147, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'WV', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(148, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'WI', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(149, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'WY', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(150, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'DC', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(152, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'AS', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(153, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'GU', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(155, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'MP', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(156, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'PR', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(157, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'VI', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(158, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'UM', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(159, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'AE', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(160, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'AA', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500),
(161, 'STANDARD', b'0', 'Somewhere', NULL, 'Sometown', 'AP', 'Other County', 'USA', '401', 'NA', 'US', 35, 75, 2500);

-- --------------------------------------------------------

--
-- Structure for view `ancestors`
--
DROP TABLE IF EXISTS `ancestors`;

CREATE ALGORITHM=UNDEFINED SQL SECURITY DEFINER VIEW `ancestors`  AS WITH recursive ancestors AS (SELECT `r_industries`.`iid` AS `base`, `r_industries`.`industry` AS `baseIndustry`, `r_industries`.`iid` AS `iid`, `r_industries`.`industry` AS `industry`, `r_industries`.`parent` AS `parent` FROM `r_industries` UNION ALL SELECT `a`.`base` AS `base`, `a`.`baseIndustry` AS `baseIndustry`, `p`.`iid` AS `iid`, `p`.`industry` AS `industry`, `p`.`parent` AS `parent` FROM (`r_industries` `p` join `ancestors` `a` on(`p`.`iid` = `a`.`parent`))) SELECT `ancestors`.`base` AS `base`, `ancestors`.`baseIndustry` AS `baseIndustry`, `ancestors`.`iid` AS `ancestor`, `ancestors`.`industry` AS `ancestorIndustry` FROM `ancestors`;

-- --------------------------------------------------------

--
-- Structure for view `descendants`
--
DROP TABLE IF EXISTS `descendants`;

CREATE ALGORITHM=UNDEFINED SQL SECURITY DEFINER VIEW `descendants`  AS WITH recursive descendants AS (SELECT `r_industries`.`iid` AS `base`, `r_industries`.`industry` AS `baseIndustry`, `r_industries`.`iid` AS `iid`, `r_industries`.`industry` AS `industry` FROM `r_industries` UNION ALL SELECT `d`.`base` AS `base`, `d`.`baseIndustry` AS `baseIndustry`, `c`.`iid` AS `iid`, `c`.`industry` AS `industry` FROM (`r_industries` `c` join `descendants` `d` on(`d`.`iid` = `c`.`parent`))) SELECT `descendants`.`base` AS `base`, `descendants`.`baseIndustry` AS `baseIndustry`, `descendants`.`iid` AS `descendant`, `descendants`.`industry` AS `descendantIndustry` FROM `descendants`;

-- --------------------------------------------------------

--
-- Structure for view `txs`
--
DROP TABLE IF EXISTS `txs`;

CREATE ALGORITHM=UNDEFINED SQL SECURITY DEFINER VIEW `txs`  AS SELECT DISTINCT `t`.`xid` AS `xid`, `t`.`actorId` AS `actorId`, `t`.`actorAgentId` AS `actorAgentId`, `t`.`flags` AS `flags`, `t`.`channel` AS `channel`, `t`.`boxId` AS `boxId`, `t`.`goods` AS `goods`, `t`.`risk` AS `risk`, `t`.`risks` AS `risks`, `t`.`recursId` AS `recursId`, `t`.`reversesXid` AS `reversesXid`, `t`.`created` AS `created`, `e2`.`entryType` AS `type`, `e2`.`amount` AS `amt`, `e2`.`rule` AS `rule`, `e2`.`relType` AS `relType`, `e2`.`relatedId` AS `rel`, `e2`.`id` AS `eid`, `e2`.`description` AS `for2`, `e2`.`uid` AS `uid2`, `e2`.`agentUid` AS `agt2`, `e2`.`cat` AS `cat2`, `e1`.`description` AS `for1`, `e1`.`uid` AS `uid1`, `e1`.`agentUid` AS `agt1`, `e1`.`cat` AS `cat1` FROM ((`tx_hdrs` `t` join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 1 and `e`.`id` < 0 or `e`.`entryType` = 2 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` < 0 or `e`.`entryType` = 3 and `e2`.`entryType` = 1 and `e2`.`id` < 0 or `e`.`entryType` = 4 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` < 0 or `e`.`entryType` = 5 and `e`.`uid` in (256,257) and `e2`.`uid` not in (256,257) or `e`.`entryType` = 7 and `e`.`uid` = 255 and `e2`.`uid` <> 255) `e1` on(`t`.`xid` = `e1`.`xid`)) join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 1 and `e`.`id` > 0 or `e`.`entryType` = 2 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` > 0 or `e`.`entryType` = 3 and `e2`.`entryType` = 1 and `e2`.`id` > 0 or `e`.`entryType` = 4 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` > 0 or `e`.`entryType` = 5 and `e`.`uid` not in (256,257) and `e2`.`uid` in (256,257) or `e`.`entryType` = 7 and `e`.`uid` <> 255 and `e2`.`uid` = 255) `e2` on(`t`.`xid` = `e2`.`xid`)) WHERE `e1`.`id` = -`e2`.`id` AND `e2`.`id` > 0 ;

-- --------------------------------------------------------

--
-- Structure for view `txs2_bank`
--
DROP TABLE IF EXISTS `txs2_bank`;

CREATE ALGORITHM=UNDEFINED SQL SECURITY DEFINER VIEW `txs2_bank`  AS SELECT `txs2`.`txid` AS `txid`, `txs2`.`amount` AS `amount`, `txs2`.`payee` AS `payee`, `txs2`.`created` AS `created`, `txs2`.`completed` AS `completed`, `txs2`.`deposit` AS `deposit`, `txs2`.`bankAccount` AS `bankAccount`, `txs2`.`isSavings` AS `isSavings`, `txs2`.`risk` AS `risk`, `txs2`.`risks` AS `risks`, `txs2`.`bankTxId` AS `bankTxId`, `txs2`.`channel` AS `channel`, `txs2`.`xid` AS `xid`, `txs2`.`pid` AS `pid` FROM `txs2` WHERE `txs2`.`pid` is null ;

-- --------------------------------------------------------

--
-- Structure for view `txs2_outer`
--
DROP TABLE IF EXISTS `txs2_outer`;

CREATE ALGORITHM=UNDEFINED SQL SECURITY DEFINER VIEW `txs2_outer`  AS SELECT `txs2`.`txid` AS `txid`, `txs2`.`amount` AS `amount`, `txs2`.`payee` AS `payee`, `txs2`.`created` AS `created`, `txs2`.`completed` AS `completed`, `txs2`.`deposit` AS `deposit`, `txs2`.`bankAccount` AS `bankAccount`, `txs2`.`isSavings` AS `isSavings`, `txs2`.`risk` AS `risk`, `txs2`.`risks` AS `risks`, `txs2`.`bankTxId` AS `bankTxId`, `txs2`.`channel` AS `channel`, `txs2`.`xid` AS `xid`, `txs2`.`pid` AS `pid` FROM `txs2` WHERE `txs2`.`pid` is not null ;

-- --------------------------------------------------------

--
-- Structure for view `txs_all`
--
DROP TABLE IF EXISTS `txs_all`;

CREATE ALGORITHM=UNDEFINED SQL SECURITY DEFINER VIEW `txs_all`  AS SELECT DISTINCT `t`.`xid` AS `xid`, `t`.`actorId` AS `actorId`, `t`.`actorAgentId` AS `actorAgentId`, `t`.`flags` AS `flags`, `t`.`channel` AS `channel`, `t`.`boxId` AS `boxId`, `t`.`goods` AS `goods`, `t`.`risk` AS `risk`, `t`.`risks` AS `risks`, `t`.`recursId` AS `recursId`, `t`.`reversesXid` AS `reversesXid`, `t`.`created` AS `created`, `e2`.`entryType` AS `type`, `e2`.`amount` AS `amt`, `e2`.`rule` AS `rule`, `e2`.`relType` AS `relType`, `e2`.`relatedId` AS `rel`, `e2`.`id` AS `eid`, `e2`.`description` AS `for2`, `e2`.`uid` AS `uid2`, `e2`.`agentUid` AS `agt2`, `e2`.`cat` AS `cat2`, `e1`.`description` AS `for1`, `e1`.`uid` AS `uid1`, `e1`.`agentUid` AS `agt1`, `e1`.`cat` AS `cat1` FROM ((`tx_hdrs` `t` join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 1 and `e`.`id` < 0 or `e`.`entryType` = 2 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` < 0 or `e`.`entryType` = 3 and `e2`.`entryType` = 1 and `e2`.`id` < 0 or `e`.`entryType` = 4 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` < 0 or `e`.`entryType` = 5 and `e`.`uid` in (256,257) and `e2`.`uid` not in (256,257) or `e`.`entryType` = 7 and `e`.`uid` = 255 and `e2`.`uid` <> 255 or `e`.`entryType` = 0 and if(`e`.`uid` in (256,257) and `e2`.`uid` in (256,257),`e`.`amount` < 0,`e`.`uid` in (256,257))) `e1` on(`t`.`xid` = `e1`.`xid`)) join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 1 and `e`.`id` > 0 or `e`.`entryType` = 2 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` > 0 or `e`.`entryType` = 3 and `e2`.`entryType` = 1 and `e2`.`id` > 0 or `e`.`entryType` = 4 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` > 0 or `e`.`entryType` = 5 and `e`.`uid` not in (256,257) and `e2`.`uid` in (256,257) or `e`.`entryType` = 7 and `e`.`uid` <> 255 and `e2`.`uid` = 255 or `e`.`entryType` = 0 and if(`e`.`uid` in (256,257) and `e2`.`uid` in (256,257),`e`.`amount` > 0,`e2`.`uid` in (256,257))) `e2` on(`t`.`xid` = `e2`.`xid`)) WHERE `e1`.`id` = -`e2`.`id` AND `e2`.`id` > 0 ;

-- --------------------------------------------------------

--
-- Structure for view `txs_aux`
--
DROP TABLE IF EXISTS `txs_aux`;

CREATE ALGORITHM=UNDEFINED SQL SECURITY DEFINER VIEW `txs_aux`  AS SELECT DISTINCT `t`.`xid` AS `xid`, `t`.`actorId` AS `actorId`, `t`.`actorAgentId` AS `actorAgentId`, `t`.`flags` AS `flags`, `t`.`channel` AS `channel`, `t`.`boxId` AS `boxId`, `t`.`goods` AS `goods`, `t`.`risk` AS `risk`, `t`.`risks` AS `risks`, `t`.`recursId` AS `recursId`, `t`.`reversesXid` AS `reversesXid`, `t`.`created` AS `created`, `e2`.`entryType` AS `type`, `e2`.`amount` AS `amt`, `e2`.`rule` AS `rule`, `e2`.`relType` AS `relType`, `e2`.`relatedId` AS `rel`, `e2`.`id` AS `eid`, `e2`.`description` AS `for2`, `e2`.`uid` AS `uid2`, `e2`.`agentUid` AS `agt2`, `e2`.`cat` AS `cat2`, `e1`.`description` AS `for1`, `e1`.`uid` AS `uid1`, `e1`.`agentUid` AS `agt1`, `e1`.`cat` AS `cat1` FROM ((`tx_hdrs` `t` join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 2 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` < 0) `e1` on(`t`.`xid` = `e1`.`xid`)) join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 2 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` > 0) `e2` on(`t`.`xid` = `e2`.`xid`)) WHERE `e1`.`id` = -`e2`.`id` AND `e2`.`id` > 0 ;

-- --------------------------------------------------------

--
-- Structure for view `txs_bank`
--
DROP TABLE IF EXISTS `txs_bank`;

CREATE ALGORITHM=UNDEFINED SQL SECURITY DEFINER VIEW `txs_bank`  AS SELECT DISTINCT `t`.`xid` AS `xid`, `t`.`actorId` AS `actorId`, `t`.`actorAgentId` AS `actorAgentId`, `t`.`flags` AS `flags`, `t`.`channel` AS `channel`, `t`.`boxId` AS `boxId`, `t`.`goods` AS `goods`, `t`.`risk` AS `risk`, `t`.`risks` AS `risks`, `t`.`recursId` AS `recursId`, `t`.`reversesXid` AS `reversesXid`, `t`.`created` AS `created`, `e2`.`entryType` AS `type`, `e2`.`amount` AS `amt`, `e2`.`rule` AS `rule`, `e2`.`relType` AS `relType`, `e2`.`relatedId` AS `rel`, `e2`.`id` AS `eid`, `e2`.`description` AS `for2`, `e2`.`uid` AS `uid2`, `e2`.`agentUid` AS `agt2`, `e2`.`cat` AS `cat2`, `e1`.`description` AS `for1`, `e1`.`uid` AS `uid1`, `e1`.`agentUid` AS `agt1`, `e1`.`cat` AS `cat1` FROM ((`tx_hdrs` `t` join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 5 and `e`.`uid` in (256,257) and `e2`.`uid` not in (256,257)) `e1` on(`t`.`xid` = `e1`.`xid`)) join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 5 and `e`.`uid` not in (256,257) and `e2`.`uid` in (256,257)) `e2` on(`t`.`xid` = `e2`.`xid`)) WHERE `e1`.`id` = -`e2`.`id` AND `e2`.`id` > 0 ;

-- --------------------------------------------------------

--
-- Structure for view `txs_bank_only`
--
DROP TABLE IF EXISTS `txs_bank_only`;

CREATE ALGORITHM=UNDEFINED SQL SECURITY DEFINER VIEW `txs_bank_only`  AS SELECT DISTINCT `t`.`xid` AS `xid`, `t`.`actorId` AS `actorId`, `t`.`actorAgentId` AS `actorAgentId`, `t`.`flags` AS `flags`, `t`.`channel` AS `channel`, `t`.`boxId` AS `boxId`, `t`.`goods` AS `goods`, `t`.`risk` AS `risk`, `t`.`risks` AS `risks`, `t`.`recursId` AS `recursId`, `t`.`reversesXid` AS `reversesXid`, `t`.`created` AS `created`, `e2`.`entryType` AS `type`, `e2`.`amount` AS `amt`, `e2`.`rule` AS `rule`, `e2`.`relType` AS `relType`, `e2`.`relatedId` AS `rel`, `e2`.`id` AS `eid`, `e2`.`description` AS `for2`, `e2`.`uid` AS `uid2`, `e2`.`agentUid` AS `agt2`, `e2`.`cat` AS `cat2`, `e1`.`description` AS `for1`, `e1`.`uid` AS `uid1`, `e1`.`agentUid` AS `agt1`, `e1`.`cat` AS `cat1` FROM ((`tx_hdrs` `t` join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 0 and if(`e`.`uid` in (256,257) and `e2`.`uid` in (256,257),`e`.`amount` < 0,`e`.`uid` in (256,257))) `e1` on(`t`.`xid` = `e1`.`xid`)) join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 0 and if(`e`.`uid` in (256,257) and `e2`.`uid` in (256,257),`e`.`amount` > 0,`e2`.`uid` in (256,257))) `e2` on(`t`.`xid` = `e2`.`xid`)) WHERE `e1`.`id` = -`e2`.`id` AND `e2`.`id` > 0 ;

-- --------------------------------------------------------

--
-- Structure for view `txs_ever`
--
DROP TABLE IF EXISTS `txs_ever`;

CREATE ALGORITHM=UNDEFINED SQL SECURITY DEFINER VIEW `txs_ever`  AS SELECT DISTINCT `t`.`xid` AS `xid`, `t`.`actorId` AS `actorId`, `t`.`actorAgentId` AS `actorAgentId`, `t`.`flags` AS `flags`, `t`.`channel` AS `channel`, `t`.`boxId` AS `boxId`, `t`.`goods` AS `goods`, `t`.`risk` AS `risk`, `t`.`risks` AS `risks`, `t`.`recursId` AS `recursId`, `t`.`reversesXid` AS `reversesXid`, `t`.`created` AS `created`, `e2`.`entryType` AS `type`, `e2`.`amount` AS `amt`, `e2`.`rule` AS `rule`, `e2`.`relType` AS `relType`, `e2`.`relatedId` AS `rel`, `e2`.`id` AS `eid`, `e2`.`description` AS `for2`, `e2`.`uid` AS `uid2`, `e2`.`agentUid` AS `agt2`, `e2`.`cat` AS `cat2`, `e1`.`description` AS `for1`, `e1`.`uid` AS `uid1`, `e1`.`agentUid` AS `agt1`, `e1`.`cat` AS `cat1` FROM ((`tx_hdrs` `t` join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 1 and `e`.`id` < 0 or `e`.`entryType` = 2 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` < 0 or `e`.`entryType` = 3 and `e2`.`entryType` = 1 and `e2`.`id` < 0 or `e`.`entryType` = 4 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` < 0 or `e`.`entryType` = 5 and `e`.`uid` in (256,257) and `e2`.`uid` not in (256,257) or `e`.`entryType` = 7 and `e`.`uid` = 255 and `e2`.`uid` <> 255 or `e`.`entryType` = 0 and if(`e`.`uid` in (256,257) and `e2`.`uid` in (256,257),`e`.`amount` < 0,`e`.`uid` in (256,257))) `e1` on(`t`.`xid` = `e1`.`xid`)) join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 1 and `e`.`id` > 0 or `e`.`entryType` = 2 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` > 0 or `e`.`entryType` = 3 and `e2`.`entryType` = 1 and `e2`.`id` > 0 or `e`.`entryType` = 4 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` > 0 or `e`.`entryType` = 5 and `e`.`uid` not in (256,257) and `e2`.`uid` in (256,257) or `e`.`entryType` = 7 and `e`.`uid` <> 255 and `e2`.`uid` = 255 or `e`.`entryType` = 0 and if(`e`.`uid` in (256,257) and `e2`.`uid` in (256,257),`e`.`amount` > 0,`e2`.`uid` in (256,257))) `e2` on(`t`.`xid` = `e2`.`xid`)) WHERE `e1`.`id` = -`e2`.`id` AND `e2`.`id` > 0 ;

-- --------------------------------------------------------

--
-- Structure for view `txs_noreverse`
--
DROP TABLE IF EXISTS `txs_noreverse`;

CREATE ALGORITHM=UNDEFINED SQL SECURITY DEFINER VIEW `txs_noreverse`  AS SELECT `t`.`xid` AS `xid`, `t`.`actorId` AS `actorId`, `t`.`actorAgentId` AS `actorAgentId`, `t`.`flags` AS `flags`, `t`.`channel` AS `channel`, `t`.`boxId` AS `boxId`, `t`.`goods` AS `goods`, `t`.`risk` AS `risk`, `t`.`risks` AS `risks`, `t`.`recursId` AS `recursId`, `t`.`reversesXid` AS `reversesXid`, `t`.`created` AS `created`, `t`.`type` AS `type`, `t`.`amt` AS `amt`, `t`.`rule` AS `rule`, `t`.`relType` AS `relType`, `t`.`rel` AS `rel`, `t`.`eid` AS `eid`, `t`.`for2` AS `for2`, `t`.`uid2` AS `uid2`, `t`.`agt2` AS `agt2`, `t`.`cat2` AS `cat2`, `t`.`for1` AS `for1`, `t`.`uid1` AS `uid1`, `t`.`agt1` AS `agt1`, `t`.`cat1` AS `cat1` FROM `txs` AS `t` WHERE `t`.`reversesXid` is null AND !exists(select `tr`.`xid` from `tx_hdrs` `tr` where `tr`.`reversesXid` = `t`.`xid` limit 1) ;

-- --------------------------------------------------------

--
-- Structure for view `txs_outer`
--
DROP TABLE IF EXISTS `txs_outer`;

CREATE ALGORITHM=UNDEFINED SQL SECURITY DEFINER VIEW `txs_outer`  AS SELECT DISTINCT `t`.`xid` AS `xid`, `t`.`actorId` AS `actorId`, `t`.`actorAgentId` AS `actorAgentId`, `t`.`flags` AS `flags`, `t`.`channel` AS `channel`, `t`.`boxId` AS `boxId`, `t`.`goods` AS `goods`, `t`.`risk` AS `risk`, `t`.`risks` AS `risks`, `t`.`recursId` AS `recursId`, `t`.`reversesXid` AS `reversesXid`, `t`.`created` AS `created`, `e2`.`entryType` AS `type`, `e2`.`amount` AS `amt`, `e2`.`rule` AS `rule`, `e2`.`relType` AS `relType`, `e2`.`relatedId` AS `rel`, `e2`.`id` AS `eid`, `e2`.`description` AS `for2`, `e2`.`uid` AS `uid2`, `e2`.`agentUid` AS `agt2`, `e2`.`cat` AS `cat2`, `e1`.`description` AS `for1`, `e1`.`uid` AS `uid1`, `e1`.`agentUid` AS `agt1`, `e1`.`cat` AS `cat1` FROM ((`tx_hdrs` `t` join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 7 and `e`.`uid` = 255 and `e2`.`uid` <> 255) `e1` on(`t`.`xid` = `e1`.`xid`)) join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 7 and `e`.`uid` <> 255 and `e2`.`uid` = 255) `e2` on(`t`.`xid` = `e2`.`xid`)) WHERE `e1`.`id` = -`e2`.`id` AND `e2`.`id` > 0 ;

-- --------------------------------------------------------

--
-- Structure for view `txs_prime`
--
DROP TABLE IF EXISTS `txs_prime`;

CREATE ALGORITHM=UNDEFINED SQL SECURITY DEFINER VIEW `txs_prime`  AS SELECT DISTINCT `t`.`xid` AS `xid`, `t`.`actorId` AS `actorId`, `t`.`actorAgentId` AS `actorAgentId`, `t`.`flags` AS `flags`, `t`.`channel` AS `channel`, `t`.`boxId` AS `boxId`, `t`.`goods` AS `goods`, `t`.`risk` AS `risk`, `t`.`risks` AS `risks`, `t`.`recursId` AS `recursId`, `t`.`reversesXid` AS `reversesXid`, `t`.`created` AS `created`, `e2`.`entryType` AS `type`, `e2`.`amount` AS `amt`, `e2`.`rule` AS `rule`, `e2`.`relType` AS `relType`, `e2`.`relatedId` AS `rel`, `e2`.`id` AS `eid`, `e2`.`description` AS `for2`, `e2`.`uid` AS `uid2`, `e2`.`agentUid` AS `agt2`, `e2`.`cat` AS `cat2`, `e1`.`description` AS `for1`, `e1`.`uid` AS `uid1`, `e1`.`agentUid` AS `agt1`, `e1`.`cat` AS `cat1` FROM ((`tx_hdrs` `t` join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 1 and `e`.`id` < 0) `e1` on(`t`.`xid` = `e1`.`xid`)) join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 1 and `e`.`id` > 0) `e2` on(`t`.`xid` = `e2`.`xid`)) WHERE `e1`.`id` = -`e2`.`id` AND `e2`.`id` > 0 ;

-- --------------------------------------------------------

--
-- Structure for view `txs_proper`
--
DROP TABLE IF EXISTS `txs_proper`;

CREATE ALGORITHM=UNDEFINED SQL SECURITY DEFINER VIEW `txs_proper`  AS SELECT DISTINCT `t`.`xid` AS `xid`, `t`.`actorId` AS `actorId`, `t`.`actorAgentId` AS `actorAgentId`, `t`.`flags` AS `flags`, `t`.`channel` AS `channel`, `t`.`boxId` AS `boxId`, `t`.`goods` AS `goods`, `t`.`risk` AS `risk`, `t`.`risks` AS `risks`, `t`.`recursId` AS `recursId`, `t`.`reversesXid` AS `reversesXid`, `t`.`created` AS `created`, `e2`.`entryType` AS `type`, `e2`.`amount` AS `amt`, `e2`.`rule` AS `rule`, `e2`.`relType` AS `relType`, `e2`.`relatedId` AS `rel`, `e2`.`id` AS `eid`, `e2`.`description` AS `for2`, `e2`.`uid` AS `uid2`, `e2`.`agentUid` AS `agt2`, `e2`.`cat` AS `cat2`, `e1`.`description` AS `for1`, `e1`.`uid` AS `uid1`, `e1`.`agentUid` AS `agt1`, `e1`.`cat` AS `cat1` FROM ((`tx_hdrs` `t` join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 1 and `e`.`id` < 0 or `e`.`entryType` = 2 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` < 0 or `e`.`entryType` = 3 and `e2`.`entryType` = 1 and `e2`.`id` < 0 or `e`.`entryType` = 4 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` < 0) `e1` on(`t`.`xid` = `e1`.`xid`)) join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 1 and `e`.`id` > 0 or `e`.`entryType` = 2 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` > 0 or `e`.`entryType` = 3 and `e2`.`entryType` = 1 and `e2`.`id` > 0 or `e`.`entryType` = 4 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` > 0) `e2` on(`t`.`xid` = `e2`.`xid`)) WHERE `e1`.`id` = -`e2`.`id` AND `e2`.`id` > 0 ;

-- --------------------------------------------------------

--
-- Structure for view `txs_rebate`
--
DROP TABLE IF EXISTS `txs_rebate`;

CREATE ALGORITHM=UNDEFINED SQL SECURITY DEFINER VIEW `txs_rebate`  AS SELECT DISTINCT `t`.`xid` AS `xid`, `t`.`actorId` AS `actorId`, `t`.`actorAgentId` AS `actorAgentId`, `t`.`flags` AS `flags`, `t`.`channel` AS `channel`, `t`.`boxId` AS `boxId`, `t`.`goods` AS `goods`, `t`.`risk` AS `risk`, `t`.`risks` AS `risks`, `t`.`recursId` AS `recursId`, `t`.`reversesXid` AS `reversesXid`, `t`.`created` AS `created`, `e2`.`entryType` AS `type`, `e2`.`amount` AS `amt`, `e2`.`rule` AS `rule`, `e2`.`relType` AS `relType`, `e2`.`relatedId` AS `rel`, `e2`.`id` AS `eid`, `e2`.`description` AS `for2`, `e2`.`uid` AS `uid2`, `e2`.`agentUid` AS `agt2`, `e2`.`cat` AS `cat2`, `e1`.`description` AS `for1`, `e1`.`uid` AS `uid1`, `e1`.`agentUid` AS `agt1`, `e1`.`cat` AS `cat1` FROM ((`tx_hdrs` `t` join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 3 and `e2`.`entryType` = 1 and `e2`.`id` < 0) `e1` on(`t`.`xid` = `e1`.`xid`)) join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 3 and `e2`.`entryType` = 1 and `e2`.`id` > 0) `e2` on(`t`.`xid` = `e2`.`xid`)) WHERE `e1`.`id` = -`e2`.`id` AND `e2`.`id` > 0 ;

-- --------------------------------------------------------

--
-- Structure for view `txs_usd_fee`
--
DROP TABLE IF EXISTS `txs_usd_fee`;

CREATE ALGORITHM=UNDEFINED SQL SECURITY DEFINER VIEW `txs_usd_fee`  AS SELECT DISTINCT `t`.`xid` AS `xid`, `t`.`actorId` AS `actorId`, `t`.`actorAgentId` AS `actorAgentId`, `t`.`flags` AS `flags`, `t`.`channel` AS `channel`, `t`.`boxId` AS `boxId`, `t`.`goods` AS `goods`, `t`.`risk` AS `risk`, `t`.`risks` AS `risks`, `t`.`recursId` AS `recursId`, `t`.`reversesXid` AS `reversesXid`, `t`.`created` AS `created`, `e2`.`entryType` AS `type`, `e2`.`amount` AS `amt`, `e2`.`rule` AS `rule`, `e2`.`relType` AS `relType`, `e2`.`relatedId` AS `rel`, `e2`.`id` AS `eid`, `e2`.`description` AS `for2`, `e2`.`uid` AS `uid2`, `e2`.`agentUid` AS `agt2`, `e2`.`cat` AS `cat2`, `e1`.`description` AS `for1`, `e1`.`uid` AS `uid1`, `e1`.`agentUid` AS `agt1`, `e1`.`cat` AS `cat1` FROM ((`tx_hdrs` `t` join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 4 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` < 0) `e1` on(`t`.`xid` = `e1`.`xid`)) join (select `e`.`id` AS `id`,`e`.`xid` AS `xid`,`e`.`entryType` AS `entryType`,`e`.`amount` AS `amount`,`e`.`uid` AS `uid`,`e`.`agentUid` AS `agentUid`,`e`.`description` AS `description`,`e`.`cat` AS `cat`,`e`.`relType` AS `relType`,`e`.`relatedId` AS `relatedId`,`e`.`rule` AS `rule` from (`tx_entries` `e` join `tx_entries` `e2` on(`e`.`xid` = `e2`.`xid`)) where `e`.`entryType` = 4 and (`e2`.`entryType` = 1 or `e2`.`entryType` = 5 or `e2`.`entryType` = 7) and `e2`.`id` > 0) `e2` on(`t`.`xid` = `e2`.`xid`)) WHERE `e1`.`id` = -`e2`.`id` AND `e2`.`id` > 0 ;

-- --------------------------------------------------------

--
-- Structure for view `tx_credits`
--
DROP TABLE IF EXISTS `tx_credits`;

CREATE ALGORITHM=UNDEFINED SQL SECURITY DEFINER VIEW `tx_credits`  AS SELECT `tx_requests_all`.`nvid` AS `id`, `tx_requests_all`.`created` AS `created`, `tx_requests_all`.`payee` AS `fromUid`, `tx_requests_all`.`payer` AS `toUid`, `tx_requests_all`.`amount` AS `amount`, `tx_requests_all`.`status` AS `xid`, `tx_requests_all`.`purpose` AS `purpose` FROM `tx_requests_all` WHERE `tx_requests_all`.`amount` < 0 ;

-- --------------------------------------------------------

--
-- Structure for view `tx_disputes`
--
DROP TABLE IF EXISTS `tx_disputes`;

CREATE ALGORITHM=UNDEFINED SQL SECURITY DEFINER VIEW `tx_disputes`  AS SELECT `tx_disputes_all`.`id` AS `id`, `tx_disputes_all`.`xid` AS `xid`, `tx_disputes_all`.`uid` AS `uid`, `tx_disputes_all`.`agentUid` AS `agentUid`, `tx_disputes_all`.`reason` AS `reason`, `tx_disputes_all`.`status` AS `status`, `tx_disputes_all`.`deleted` AS `deleted` FROM `tx_disputes_all` WHERE `tx_disputes_all`.`deleted` is null ;

-- --------------------------------------------------------

--
-- Structure for view `tx_entries`
--
DROP TABLE IF EXISTS `tx_entries`;

CREATE ALGORITHM=UNDEFINED SQL SECURITY DEFINER VIEW `tx_entries`  AS SELECT `tx_entries_all`.`id` AS `id`, `tx_entries_all`.`xid` AS `xid`, `tx_entries_all`.`entryType` AS `entryType`, `tx_entries_all`.`amount` AS `amount`, `tx_entries_all`.`uid` AS `uid`, `tx_entries_all`.`agentUid` AS `agentUid`, `tx_entries_all`.`description` AS `description`, `tx_entries_all`.`cat` AS `cat`, `tx_entries_all`.`relType` AS `relType`, `tx_entries_all`.`relatedId` AS `relatedId`, `tx_entries_all`.`rule` AS `rule` FROM `tx_entries_all` WHERE `tx_entries_all`.`deleted` is null ;

-- --------------------------------------------------------

--
-- Structure for view `tx_entries_o`
--
DROP TABLE IF EXISTS `tx_entries_o`;

CREATE ALGORITHM=UNDEFINED SQL SECURITY DEFINER VIEW `tx_entries_o`  AS SELECT `tx_entries`.`id` AS `id`, `tx_entries`.`xid` AS `xid`, `tx_entries`.`entryType` AS `entryType`, `tx_entries`.`amount` AS `amount`, `tx_entries`.`uid` AS `uid`, `tx_entries`.`agentUid` AS `agentUid`, `tx_entries`.`description` AS `description`, `tx_entries`.`cat` AS `cat`, `tx_entries`.`relType` AS `relType`, `tx_entries`.`relatedId` AS `relatedId`, `tx_entries`.`rule` AS `rule` FROM `tx_entries` ORDER BY abs(`tx_entries`.`id`) ASC, `tx_entries`.`id` ASC ;

-- --------------------------------------------------------

--
-- Structure for view `tx_hdrs`
--
DROP TABLE IF EXISTS `tx_hdrs`;

CREATE ALGORITHM=UNDEFINED SQL SECURITY DEFINER VIEW `tx_hdrs`  AS SELECT `tx_hdrs_all`.`xid` AS `xid`, `tx_hdrs_all`.`actorId` AS `actorId`, `tx_hdrs_all`.`actorAgentId` AS `actorAgentId`, `tx_hdrs_all`.`flags` AS `flags`, `tx_hdrs_all`.`channel` AS `channel`, `tx_hdrs_all`.`boxId` AS `boxId`, `tx_hdrs_all`.`goods` AS `goods`, `tx_hdrs_all`.`risk` AS `risk`, `tx_hdrs_all`.`risks` AS `risks`, `tx_hdrs_all`.`recursId` AS `recursId`, `tx_hdrs_all`.`reversesXid` AS `reversesXid`, `tx_hdrs_all`.`created` AS `created` FROM `tx_hdrs_all` WHERE `tx_hdrs_all`.`deleted` is null ;

-- --------------------------------------------------------

--
-- Structure for view `tx_requests`
--
DROP TABLE IF EXISTS `tx_requests`;

CREATE ALGORITHM=UNDEFINED SQL SECURITY DEFINER VIEW `tx_requests`  AS SELECT `tx_requests_all`.`nvid` AS `nvid`, `tx_requests_all`.`status` AS `status`, `tx_requests_all`.`amount` AS `amount`, `tx_requests_all`.`payer` AS `payer`, `tx_requests_all`.`payee` AS `payee`, `tx_requests_all`.`goods` AS `goods`, `tx_requests_all`.`purpose` AS `purpose`, `tx_requests_all`.`cat` AS `cat`, `tx_requests_all`.`flags` AS `flags`, `tx_requests_all`.`data` AS `data`, `tx_requests_all`.`reversesXid` AS `reversesXid`, `tx_requests_all`.`recursId` AS `recursId`, `tx_requests_all`.`created` AS `created`, `tx_requests_all`.`deleted` AS `deleted` FROM `tx_requests_all` WHERE `tx_requests_all`.`deleted` is null AND `tx_requests_all`.`amount` > 0 ;

--
-- Indexes for dumped tables
--

--
-- Indexes for table `admins`
--
ALTER TABLE `admins`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `block`
--
ALTER TABLE `block`
  ADD PRIMARY KEY (`bid`),
  ADD UNIQUE KEY `tmd` (`theme`,`module`,`delta`),
  ADD KEY `list` (`theme`,`status`,`region`,`weight`,`module`);

--
-- Indexes for table `cache`
--
ALTER TABLE `cache`
  ADD PRIMARY KEY (`cid`),
  ADD KEY `expire` (`expire`);

--
-- Indexes for table `cache_bootstrap`
--
ALTER TABLE `cache_bootstrap`
  ADD PRIMARY KEY (`cid`),
  ADD KEY `expire` (`expire`);

--
-- Indexes for table `cache_form`
--
ALTER TABLE `cache_form`
  ADD PRIMARY KEY (`cid`),
  ADD KEY `expire` (`expire`);

--
-- Indexes for table `cache_menu`
--
ALTER TABLE `cache_menu`
  ADD PRIMARY KEY (`cid`),
  ADD KEY `expire` (`expire`);

--
-- Indexes for table `cu_folders`
--
ALTER TABLE `cu_folders`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `cu_lists`
--
ALTER TABLE `cu_lists`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `cu_members`
--
ALTER TABLE `cu_members`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `cu_spaces`
--
ALTER TABLE `cu_spaces`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `cu_tasks`
--
ALTER TABLE `cu_tasks`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `cu_times`
--
ALTER TABLE `cu_times`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `flood`
--
ALTER TABLE `flood`
  ADD PRIMARY KEY (`fid`),
  ADD KEY `allow` (`event`,`identifier`,`timestamp`),
  ADD KEY `purge` (`expiration`);

--
-- Indexes for table `legacy_x_invoices`
--
ALTER TABLE `legacy_x_invoices`
  ADD PRIMARY KEY (`nvid`,`deleted`),
  ADD KEY `payer` (`payer`),
  ADD KEY `payee` (`payee`),
  ADD KEY `created` (`created`),
  ADD KEY `status` (`status`);

--
-- Indexes for table `legacy_x_txs`
--
ALTER TABLE `legacy_x_txs`
  ADD PRIMARY KEY (`xid`,`deleted`),
  ADD KEY `payer` (`payer`),
  ADD KEY `payee` (`payee`),
  ADD KEY `created` (`created`);

--
-- Indexes for table `menu_links`
--
ALTER TABLE `menu_links`
  ADD PRIMARY KEY (`mlid`),
  ADD KEY `path_menu` (`link_path`(128),`menu_name`),
  ADD KEY `menu_plid_expand_child` (`menu_name`,`plid`,`expanded`,`has_children`),
  ADD KEY `menu_parents` (`menu_name`,`p1`,`p2`,`p3`,`p4`,`p5`,`p6`,`p7`,`p8`,`p9`),
  ADD KEY `router_path` (`router_path`(128));

--
-- Indexes for table `menu_router`
--
ALTER TABLE `menu_router`
  ADD PRIMARY KEY (`path`),
  ADD KEY `fit` (`fit`),
  ADD KEY `tab_parent` (`tab_parent`(64),`weight`,`title`),
  ADD KEY `tab_root_weight_title` (`tab_root`(64),`weight`,`title`);

--
-- Indexes for table `messages`
--
ALTER TABLE `messages`
  ADD PRIMARY KEY (`id`),
  ADD KEY `sender` (`sender`);

--
-- Indexes for table `people`
--
ALTER TABLE `people`
  ADD PRIMARY KEY (`pid`),
  ADD KEY `email` (`email`);

--
-- Indexes for table `phinxlog`
--
ALTER TABLE `phinxlog`
  ADD PRIMARY KEY (`version`);

--
-- Indexes for table `posts`
--
ALTER TABLE `posts`
  ADD PRIMARY KEY (`postid`),
  ADD KEY `pid` (`pid`),
  ADD KEY `type` (`type`),
  ADD KEY `cat` (`cat`),
  ADD KEY `created` (`created`);

--
-- Indexes for table `post_cats`
--
ALTER TABLE `post_cats`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `queue`
--
ALTER TABLE `queue`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `registry`
--
ALTER TABLE `registry`
  ADD PRIMARY KEY (`name`,`type`),
  ADD KEY `hook` (`type`,`weight`,`module`);

--
-- Indexes for table `registry_file`
--
ALTER TABLE `registry_file`
  ADD PRIMARY KEY (`filename`);

--
-- Indexes for table `r_areas`
--
ALTER TABLE `r_areas`
  ADD PRIMARY KEY (`area_code`);

--
-- Indexes for table `r_bad`
--
ALTER TABLE `r_bad`
  ADD PRIMARY KEY (`created`);

--
-- Indexes for table `r_ballots`
--
ALTER TABLE `r_ballots`
  ADD PRIMARY KEY (`id`),
  ADD KEY `question` (`question`),
  ADD KEY `voter` (`voter`),
  ADD KEY `proxy` (`proxy`);

--
-- Indexes for table `r_banks`
--
ALTER TABLE `r_banks`
  ADD PRIMARY KEY (`route`),
  ADD KEY `newroute` (`newRoute`);

--
-- Indexes for table `r_boxes`
--
ALTER TABLE `r_boxes`
  ADD PRIMARY KEY (`id`),
  ADD KEY `uid` (`uid`);

--
-- Indexes for table `r_changes`
--
ALTER TABLE `r_changes`
  ADD PRIMARY KEY (`id`),
  ADD KEY `uid` (`uid`);

--
-- Indexes for table `r_countries`
--
ALTER TABLE `r_countries`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `name_iso_code` (`name`,`iso_code`),
  ADD KEY `address_format_id` (`address_format_id`),
  ADD KEY `region_id` (`region_id`);

--
-- Indexes for table `r_criteria`
--
ALTER TABLE `r_criteria`
  ADD PRIMARY KEY (`id`),
  ADD KEY `ctty` (`ctty`);

--
-- Indexes for table `r_do`
--
ALTER TABLE `r_do`
  ADD PRIMARY KEY (`doid`),
  ADD KEY `uid` (`uid`);

--
-- Indexes for table `r_events`
--
ALTER TABLE `r_events`
  ADD PRIMARY KEY (`id`),
  ADD KEY `ctty` (`ctty`);

--
-- Indexes for table `r_honors`
--
ALTER TABLE `r_honors`
  ADD PRIMARY KEY (`id`),
  ADD KEY `uid` (`uid`);

--
-- Indexes for table `r_industries`
--
ALTER TABLE `r_industries`
  ADD PRIMARY KEY (`iid`),
  ADD KEY `parent` (`parent`);

--
-- Indexes for table `r_investments`
--
ALTER TABLE `r_investments`
  ADD PRIMARY KEY (`vestid`),
  ADD KEY `coid` (`coid`),
  ADD KEY `proposedBy` (`proposedBy`);

--
-- Indexes for table `r_invites`
--
ALTER TABLE `r_invites`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `id` (`id`),
  ADD KEY `inviter` (`inviter`);

--
-- Indexes for table `r_ips`
--
ALTER TABLE `r_ips`
  ADD PRIMARY KEY (`ip`),
  ADD KEY `uid` (`uid`);

--
-- Indexes for table `r_near`
--
ALTER TABLE `r_near`
  ADD PRIMARY KEY (`uid1`,`uid2`);

--
-- Indexes for table `r_notices`
--
ALTER TABLE `r_notices`
  ADD PRIMARY KEY (`msgid`),
  ADD KEY `uid` (`uid`);

--
-- Indexes for table `r_options`
--
ALTER TABLE `r_options`
  ADD PRIMARY KEY (`id`),
  ADD KEY `question` (`question`);

--
-- Indexes for table `r_pairs`
--
ALTER TABLE `r_pairs`
  ADD PRIMARY KEY (`id`),
  ADD KEY `option1` (`option1`),
  ADD KEY `option2` (`option2`);

--
-- Indexes for table `r_proposals`
--
ALTER TABLE `r_proposals`
  ADD PRIMARY KEY (`id`),
  ADD KEY `ctty` (`ctty`),
  ADD KEY `name` (`name`);

--
-- Indexes for table `r_proxies`
--
ALTER TABLE `r_proxies`
  ADD PRIMARY KEY (`id`),
  ADD KEY `person` (`person`);

--
-- Indexes for table `r_questions`
--
ALTER TABLE `r_questions`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `r_ratings`
--
ALTER TABLE `r_ratings`
  ADD PRIMARY KEY (`ratingid`),
  ADD KEY `vestid` (`vestid`),
  ADD KEY `uid` (`uid`);

--
-- Indexes for table `r_regions`
--
ALTER TABLE `r_regions`
  ADD PRIMARY KEY (`region`),
  ADD UNIQUE KEY `fullName` (`fullName`),
  ADD KEY `state` (`st`);

--
-- Indexes for table `r_shares`
--
ALTER TABLE `r_shares`
  ADD PRIMARY KEY (`shid`),
  ADD KEY `vestid` (`vestid`);

--
-- Indexes for table `r_stakes`
--
ALTER TABLE `r_stakes`
  ADD PRIMARY KEY (`stakeid`),
  ADD KEY `uid` (`uid`);

--
-- Indexes for table `r_states`
--
ALTER TABLE `r_states`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `name_country_id` (`name`,`country_id`),
  ADD KEY `country_id` (`country_id`);

--
-- Indexes for table `r_stats`
--
ALTER TABLE `r_stats`
  ADD PRIMARY KEY (`id`),
  ADD KEY `ctty` (`ctty`);

--
-- Indexes for table `r_tous`
--
ALTER TABLE `r_tous`
  ADD PRIMARY KEY (`id`),
  ADD KEY `uid` (`uid`);

--
-- Indexes for table `r_transit`
--
ALTER TABLE `r_transit`
  ADD PRIMARY KEY (`location`);

--
-- Indexes for table `r_usd2`
--
ALTER TABLE `r_usd2`
  ADD PRIMARY KEY (`id`),
  ADD KEY `completed` (`completed`);

--
-- Indexes for table `r_user_industries`
--
ALTER TABLE `r_user_industries`
  ADD PRIMARY KEY (`id`),
  ADD KEY `uid` (`uid`),
  ADD KEY `iid` (`iid`);

--
-- Indexes for table `r_votes`
--
ALTER TABLE `r_votes`
  ADD PRIMARY KEY (`id`),
  ADD KEY `ballot` (`ballot`),
  ADD KEY `option` (`option`);

--
-- Indexes for table `semaphore`
--
ALTER TABLE `semaphore`
  ADD PRIMARY KEY (`name`),
  ADD KEY `value` (`value`),
  ADD KEY `expire` (`expire`);

--
-- Indexes for table `sessions`
--
ALTER TABLE `sessions`
  ADD PRIMARY KEY (`sid`,`ssid`),
  ADD KEY `timestamp` (`timestamp`),
  ADD KEY `uid` (`uid`),
  ADD KEY `ssid` (`ssid`);

--
-- Indexes for table `system`
--
ALTER TABLE `system`
  ADD PRIMARY KEY (`filename`),
  ADD KEY `system_list` (`status`,`bootstrap`,`type`,`weight`,`name`),
  ADD KEY `type_name` (`type`,`name`);

--
-- Indexes for table `test`
--
ALTER TABLE `test`
  ADD PRIMARY KEY (`id`),
  ADD KEY `test` (`test`),
  ADD KEY `type` (`type`);

--
-- Indexes for table `txs2`
--
ALTER TABLE `txs2`
  ADD PRIMARY KEY (`txid`),
  ADD KEY `created` (`created`),
  ADD KEY `pid` (`pid`);

--
-- Indexes for table `tx_bads`
--
ALTER TABLE `tx_bads`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `tx_cats`
--
ALTER TABLE `tx_cats`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `tx_disputes_all`
--
ALTER TABLE `tx_disputes_all`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `tx_entries_all`
--
ALTER TABLE `tx_entries_all`
  ADD PRIMARY KEY (`id`),
  ADD KEY `xid` (`xid`),
  ADD KEY `uid` (`uid`);

--
-- Indexes for table `tx_hdrs_all`
--
ALTER TABLE `tx_hdrs_all`
  ADD PRIMARY KEY (`xid`),
  ADD UNIQUE KEY `xid` (`xid`),
  ADD UNIQUE KEY `reversesXid` (`reversesXid`),
  ADD KEY `actorId` (`actorId`),
  ADD KEY `created` (`created`),
  ADD KEY `recursId` (`recursId`);

--
-- Indexes for table `tx_requests_all`
--
ALTER TABLE `tx_requests_all`
  ADD PRIMARY KEY (`nvid`),
  ADD KEY `payer` (`payer`),
  ADD KEY `payee` (`payee`),
  ADD KEY `created` (`created`),
  ADD KEY `status` (`status`);

--
-- Indexes for table `tx_rules`
--
ALTER TABLE `tx_rules`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `tx_timed`
--
ALTER TABLE `tx_timed`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `users`
--
ALTER TABLE `users`
  ADD PRIMARY KEY (`uid`),
  ADD KEY `access` (`access`),
  ADD KEY `created` (`created`),
  ADD KEY `mail` (`email`),
  ADD KEY `picture` (`picture`),
  ADD KEY `name` (`name`);

--
-- Indexes for table `u_company`
--
ALTER TABLE `u_company`
  ADD PRIMARY KEY (`uid`);

--
-- Indexes for table `u_groupies`
--
ALTER TABLE `u_groupies`
  ADD PRIMARY KEY (`id`),
  ADD KEY `uid` (`uid`,`start`),
  ADD KEY `grpId` (`grpId`,`start`);

--
-- Indexes for table `u_groups`
--
ALTER TABLE `u_groups`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `name` (`name`);

--
-- Indexes for table `u_photo`
--
ALTER TABLE `u_photo`
  ADD PRIMARY KEY (`uid`);

--
-- Indexes for table `u_relations`
--
ALTER TABLE `u_relations`
  ADD PRIMARY KEY (`reid`),
  ADD KEY `main` (`main`),
  ADD KEY `other` (`other`);

--
-- Indexes for table `u_shout`
--
ALTER TABLE `u_shout`
  ADD PRIMARY KEY (`uid`),
  ADD UNIQUE KEY `uid` (`uid`);

--
-- Indexes for table `u_track`
--
ALTER TABLE `u_track`
  ADD PRIMARY KEY (`id`),
  ADD KEY `uid` (`uid`),
  ADD KEY `type` (`type`);

--
-- Indexes for table `variable`
--
ALTER TABLE `variable`
  ADD PRIMARY KEY (`name`);

--
-- Indexes for table `x_company`
--
ALTER TABLE `x_company`
  ADD PRIMARY KEY (`uid`);

--
-- Indexes for table `x_photo`
--
ALTER TABLE `x_photo`
  ADD PRIMARY KEY (`uid`,`deleted`);

--
-- Indexes for table `x_relations`
--
ALTER TABLE `x_relations`
  ADD PRIMARY KEY (`reid`,`deleted`),
  ADD KEY `main` (`main`),
  ADD KEY `other` (`other`);

--
-- Indexes for table `x_shout`
--
ALTER TABLE `x_shout`
  ADD PRIMARY KEY (`uid`),
  ADD UNIQUE KEY `uid` (`uid`);

--
-- Indexes for table `x_txs2`
--
ALTER TABLE `x_txs2`
  ADD PRIMARY KEY (`txid`,`deleted`),
  ADD KEY `created` (`created`),
  ADD KEY `pid` (`pid`);

--
-- Indexes for table `x_users`
--
ALTER TABLE `x_users`
  ADD PRIMARY KEY (`uid`,`deleted`),
  ADD KEY `access` (`access`),
  ADD KEY `created` (`created`),
  ADD KEY `mail` (`email`),
  ADD KEY `picture` (`picture`),
  ADD KEY `name` (`name`);

--
-- Indexes for table `zip3`
--
ALTER TABLE `zip3`
  ADD PRIMARY KEY (`id`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `admins`
--
ALTER TABLE `admins`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `block`
--
ALTER TABLE `block`
  MODIFY `bid` int(11) NOT NULL AUTO_INCREMENT COMMENT 'Primary Key: Unique block ID.', AUTO_INCREMENT=427;

--
-- AUTO_INCREMENT for table `flood`
--
ALTER TABLE `flood`
  MODIFY `fid` int(11) NOT NULL AUTO_INCREMENT COMMENT 'Unique flood event ID.', AUTO_INCREMENT=2237;

--
-- AUTO_INCREMENT for table `legacy_x_invoices`
--
ALTER TABLE `legacy_x_invoices`
  MODIFY `nvid` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'the unique invoice ID', AUTO_INCREMENT=952;

--
-- AUTO_INCREMENT for table `legacy_x_txs`
--
ALTER TABLE `legacy_x_txs`
  MODIFY `xid` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'the unique transaction ID', AUTO_INCREMENT=103273;

--
-- AUTO_INCREMENT for table `menu_links`
--
ALTER TABLE `menu_links`
  MODIFY `mlid` int(10) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT 'The menu link ID (mlid) is the integer primary key.';

--
-- AUTO_INCREMENT for table `messages`
--
ALTER TABLE `messages`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `people`
--
ALTER TABLE `people`
  MODIFY `pid` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'record ID';

--
-- AUTO_INCREMENT for table `posts`
--
ALTER TABLE `posts`
  MODIFY `postid` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'record ID';

--
-- AUTO_INCREMENT for table `post_cats`
--
ALTER TABLE `post_cats`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=22;

--
-- AUTO_INCREMENT for table `queue`
--
ALTER TABLE `queue`
  MODIFY `id` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'primary key: Unique item ID';

--
-- AUTO_INCREMENT for table `r_ballots`
--
ALTER TABLE `r_ballots`
  MODIFY `id` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'ballot record id', AUTO_INCREMENT=257489;

--
-- AUTO_INCREMENT for table `r_boxes`
--
ALTER TABLE `r_boxes`
  MODIFY `id` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'device record id';

--
-- AUTO_INCREMENT for table `r_changes`
--
ALTER TABLE `r_changes`
  MODIFY `id` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'change record ID';

--
-- AUTO_INCREMENT for table `r_countries`
--
ALTER TABLE `r_countries`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT 'Country Id', AUTO_INCREMENT=1247;

--
-- AUTO_INCREMENT for table `r_criteria`
--
ALTER TABLE `r_criteria`
  MODIFY `id` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'criterion record id', AUTO_INCREMENT=15;

--
-- AUTO_INCREMENT for table `r_do`
--
ALTER TABLE `r_do`
  MODIFY `doid` int(11) NOT NULL AUTO_INCREMENT COMMENT 'record id';

--
-- AUTO_INCREMENT for table `r_events`
--
ALTER TABLE `r_events`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT COMMENT 'record id', AUTO_INCREMENT=8;

--
-- AUTO_INCREMENT for table `r_honors`
--
ALTER TABLE `r_honors`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT COMMENT 'record id', AUTO_INCREMENT=270;

--
-- AUTO_INCREMENT for table `r_industries`
--
ALTER TABLE `r_industries`
  MODIFY `iid` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=79;

--
-- AUTO_INCREMENT for table `r_investments`
--
ALTER TABLE `r_investments`
  MODIFY `vestid` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'record ID', AUTO_INCREMENT=2;

--
-- AUTO_INCREMENT for table `r_invites`
--
ALTER TABLE `r_invites`
  MODIFY `id` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'record id';

--
-- AUTO_INCREMENT for table `r_notices`
--
ALTER TABLE `r_notices`
  MODIFY `msgid` int(11) NOT NULL AUTO_INCREMENT COMMENT 'notice record id';

--
-- AUTO_INCREMENT for table `r_options`
--
ALTER TABLE `r_options`
  MODIFY `id` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'option record id';

--
-- AUTO_INCREMENT for table `r_pairs`
--
ALTER TABLE `r_pairs`
  MODIFY `id` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'pairs record id';

--
-- AUTO_INCREMENT for table `r_proposals`
--
ALTER TABLE `r_proposals`
  MODIFY `id` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'proposal record id', AUTO_INCREMENT=66;

--
-- AUTO_INCREMENT for table `r_proxies`
--
ALTER TABLE `r_proxies`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT COMMENT 'record id';

--
-- AUTO_INCREMENT for table `r_questions`
--
ALTER TABLE `r_questions`
  MODIFY `id` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'question record id', AUTO_INCREMENT=14;

--
-- AUTO_INCREMENT for table `r_ratings`
--
ALTER TABLE `r_ratings`
  MODIFY `ratingid` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'record ID';

--
-- AUTO_INCREMENT for table `r_shares`
--
ALTER TABLE `r_shares`
  MODIFY `shid` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'record ID', AUTO_INCREMENT=3;

--
-- AUTO_INCREMENT for table `r_stakes`
--
ALTER TABLE `r_stakes`
  MODIFY `stakeid` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'record ID', AUTO_INCREMENT=3;

--
-- AUTO_INCREMENT for table `r_states`
--
ALTER TABLE `r_states`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT 'State / Province ID', AUTO_INCREMENT=10057;

--
-- AUTO_INCREMENT for table `r_stats`
--
ALTER TABLE `r_stats`
  MODIFY `id` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'statistics record id';

--
-- AUTO_INCREMENT for table `r_tous`
--
ALTER TABLE `r_tous`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT COMMENT 'vote record id', AUTO_INCREMENT=66;

--
-- AUTO_INCREMENT for table `r_usd2`
--
ALTER TABLE `r_usd2`
  MODIFY `id` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'record ID', AUTO_INCREMENT=145;

--
-- AUTO_INCREMENT for table `r_user_industries`
--
ALTER TABLE `r_user_industries`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT COMMENT 'user industry record id', AUTO_INCREMENT=367;

--
-- AUTO_INCREMENT for table `r_votes`
--
ALTER TABLE `r_votes`
  MODIFY `id` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'vote record id', AUTO_INCREMENT=16989;

--
-- AUTO_INCREMENT for table `test`
--
ALTER TABLE `test`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `txs2`
--
ALTER TABLE `txs2`
  MODIFY `txid` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'the unique transaction ID';

--
-- AUTO_INCREMENT for table `tx_bads`
--
ALTER TABLE `tx_bads`
  MODIFY `id` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'record ID';

--
-- AUTO_INCREMENT for table `tx_cats`
--
ALTER TABLE `tx_cats`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=14901;

--
-- AUTO_INCREMENT for table `tx_disputes_all`
--
ALTER TABLE `tx_disputes_all`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `tx_entries_all`
--
ALTER TABLE `tx_entries_all`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `tx_hdrs_all`
--
ALTER TABLE `tx_hdrs_all`
  MODIFY `xid` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'the unique transaction ID';

--
-- AUTO_INCREMENT for table `tx_requests_all`
--
ALTER TABLE `tx_requests_all`
  MODIFY `nvid` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'the unique invoice ID';

--
-- AUTO_INCREMENT for table `tx_rules`
--
ALTER TABLE `tx_rules`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `tx_timed`
--
ALTER TABLE `tx_timed`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `u_company`
--
ALTER TABLE `u_company`
  MODIFY `uid` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'account record ID', AUTO_INCREMENT=40326000000003;

--
-- AUTO_INCREMENT for table `u_groupies`
--
ALTER TABLE `u_groupies`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `u_groups`
--
ALTER TABLE `u_groups`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT for table `u_relations`
--
ALTER TABLE `u_relations`
  MODIFY `reid` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'relationship record id';

--
-- AUTO_INCREMENT for table `u_track`
--
ALTER TABLE `u_track`
  MODIFY `id` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'record ID';

--
-- AUTO_INCREMENT for table `x_company`
--
ALTER TABLE `x_company`
  MODIFY `uid` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'account record ID';

--
-- AUTO_INCREMENT for table `x_relations`
--
ALTER TABLE `x_relations`
  MODIFY `reid` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'relationship record id', AUTO_INCREMENT=40326000000006;

--
-- AUTO_INCREMENT for table `x_txs2`
--
ALTER TABLE `x_txs2`
  MODIFY `txid` bigint(20) NOT NULL AUTO_INCREMENT COMMENT 'the unique transaction ID', AUTO_INCREMENT=6131340;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
