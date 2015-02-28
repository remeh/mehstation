CREATE TABLE `platform`  (
	`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
	`name`	TEXT NOT NULL,
	`command`	TEXT NOT NULL
);
CREATE TABLE `executable` (
	`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,
	`display_name`	TEXT NOT NULL,
	`filepath`	TEXT NOT NULL,
    `platform_id` INTEGER NOT NULL
);
CREATE TABLE `mehstation` (
    `name` TEXT NOT NULL PRIMARY KEY UNIQUE,
    `value` TEXT
);
INSERT INTO mehstation VALUES ('schema', '1');
