DO $$
BEGIN
    IF NOT EXISTS (SELECT 1 FROM pg_type WHERE typname = 'precipitation_type') THEN
        CREATE TYPE precipitation_type AS ENUM (
            'none',
            'rain',
            'hail',
            'rain_and_hail'
        );
    END IF;
END$$;

create table if not exists observations(
    id serial primary key,
    time timestamp not null,
    firmware_revision smallint not null,
    station_sn text not null,
    hub_sn text not null,
    wind_lull_mph real not null,
    wind_avg_mph real not null,
    wind_gust_mph real not null,
    wind_dir_deg smallint not null,
    wind_sample_interval_secs smallint not null,
    pressure_inhg real not null,
    air_temp_f real not null,
    relative_humidity real not null,
    illuminance_lux integer not null,
    uv_index smallint not null,
    solar_radiation_wm2 integer not null,
    rain_accum_in real not null,
    precip_type precipitation_type not null,
    lightning_strike_dist_mi real not null,
    lightning_strike_count smallint not null,
    battery_volts real not null,
    report_interval smallint not null
);