INSERT INTO schema_migrations(version,name) VALUES(1,'initial_lithography_control_schema') ON DUPLICATE KEY UPDATE name=VALUES(name);
