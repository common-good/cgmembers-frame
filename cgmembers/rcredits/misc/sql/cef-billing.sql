-- phpMyAdmin SQL Dump
-- version 5.0.2
-- https://www.phpmyadmin.net/
--
-- Host: 127.0.0.1:3309
-- Generation Time: Jun 01, 2022 at 06:10 PM
-- Server version: 10.5.4-MariaDB
-- PHP Version: 7.4.9

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `cg`
--

-- --------------------------------------------------------

--
-- Table structure for table `cef`
--

DROP TABLE IF EXISTS `cef`;
CREATE TABLE IF NOT EXISTS `cef` (
  `User ID` int(8) DEFAULT NULL,
  `Username` varchar(16) DEFAULT NULL,
  `Time Entry ID` bigint(19) DEFAULT NULL,
  `Description` varchar(10) DEFAULT NULL,
  `Billable` varchar(5) DEFAULT NULL,
  `Time Labels` varchar(2) DEFAULT NULL,
  `Start` bigint(13) DEFAULT NULL,
  `Start Text` varchar(27) DEFAULT NULL,
  `Stop` bigint(13) DEFAULT NULL,
  `Stop Text` varchar(27) DEFAULT NULL,
  `Time Tracked` int(7) DEFAULT NULL,
  `Time Tracked Text` varchar(8) DEFAULT NULL,
  `Space ID` int(8) DEFAULT NULL,
  `Space Name` varchar(10) DEFAULT NULL,
  `Folder ID` int(9) DEFAULT NULL,
  `Folder Name` varchar(3) DEFAULT NULL,
  `List ID` int(9) DEFAULT NULL,
  `List Name` varchar(22) DEFAULT NULL,
  `Task ID` varchar(7) DEFAULT NULL,
  `Task Name` varchar(204) DEFAULT NULL,
  `Task Status` varchar(21) DEFAULT NULL,
  `Due Date` varchar(13) DEFAULT NULL,
  `Due Date Text` varchar(26) DEFAULT NULL,
  `Start Date` varchar(10) DEFAULT NULL,
  `Start Date Text` varchar(10) DEFAULT NULL,
  `Task Time Estimated` varchar(8) DEFAULT NULL,
  `Task Time Estimated Text` varchar(4) DEFAULT NULL,
  `Task Time Spent` int(9) DEFAULT NULL,
  `Task Time Spent Text` varchar(9) DEFAULT NULL,
  `User Total Time Estimated` varchar(8) DEFAULT NULL,
  `User Total Time Estimated Text` varchar(4) DEFAULT NULL,
  `User Total Time Tracked` int(8) DEFAULT NULL,
  `User Total Time Tracked Text` varchar(9) DEFAULT NULL,
  `Tags` varchar(14) DEFAULT NULL,
  `Checklists` varchar(2) DEFAULT NULL,
  `User Period Time Spent` int(8) DEFAULT NULL,
  `User Period Time Spent Text` varchar(8) DEFAULT NULL,
  `Date Created` bigint(13) DEFAULT NULL,
  `Date Created Text` varchar(27) DEFAULT NULL,
  `Custom Task ID` varchar(10) DEFAULT NULL,
  `Parent Task ID` varchar(7) DEFAULT NULL,
  `Class` varchar(8) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
