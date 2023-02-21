CREATE TABLE log.friendfinding (
id serial primary key,
degree int,
friend_requested int,
distance_score float(24),
hop_distance int,
diameter int,
not_alone int,
round_period float(24),
retain_time float(24),
nbr_lags char(1024),
round_count int,
global_clock float(24),
cur_msg int,
max_msg int,
uid int,
position_latitude float(24),
position_longitude float(24),
position_accuracy float(24)
);

grant insert on log.friendfinding to fcpp_web_client;
grant update on log.friendfinding_id_seq to fcpp_web_client;
