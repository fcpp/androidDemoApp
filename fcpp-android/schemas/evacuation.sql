CREATE TABLE log.evacuation (
id serial primary key,
diameter int,
evacuation_time int,
evacuation_group bool,
evacuation_done bool,
homogeneous_group bool,
traitor_free bool,
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

grant insert on log.evacuation to fcpp_web_client;
grant update on log.evacuation_id_seq to fcpp_web_client;