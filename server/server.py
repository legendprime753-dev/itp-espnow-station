services:
  mariadb:
    image: mariadb:latest
    container_name: mariadb
    restart: unless-stopped
    environment:
      MARIADB_ROOT_PASSWORD: rootpass
      MARIADB_DATABASE: station
      MARIADB_USER: espuser
      MARIADB_PASSWORD: esp1234
    ports:
      - "3306:3306"
    volumes:
      - mariadb_data:/var/lib/mysql

  adminer:
    image: adminer
    container_name: adminer
    restart: unless-stopped
    ports:
      - "8080:8080"

volumes:
  mariadb_data:
