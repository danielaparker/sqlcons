DROP SEQUENCE instrument_id_sequence
go
CREATE SEQUENCE instrument_id_sequence AS BIGINT
    START WITH   1  
    INCREMENT BY 1  
    MINVALUE  1     
    NO CYCLE        
    CACHE 500       
go

DROP TABLE equity
go
CREATE TABLE equity
(
    instrument_id BIGINT NOT NULL PRIMARY KEY,
    ticker NVARCHAR(30) NOT NULL,
    CONSTRAINT equity_ak UNIQUE CLUSTERED (ticker) 
)
go

DROP TABLE equity_price
go
CREATE TABLE equity_price
(
    instrument_id BIGINT NOT NULL,
    observation_date DATE NOT NULL,
    price DECIMAL(20,6) NOT NULL,
    CONSTRAINT equity_price_pk PRIMARY KEY CLUSTERED (instrument_id ASC,observation_date DESC) 
        ON [PRIMARY]
)
go

