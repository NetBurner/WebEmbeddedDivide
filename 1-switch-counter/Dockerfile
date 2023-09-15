FROM php:8.2-alpine3.18
RUN apk update \
    && apk add --no-cache \
    apache2 \
    apache2-utils \
    php82-apache2 \
    php82-sqlite3

ENV APACHE_RUN_USER www-data
ENV APACHE_RUN_GROUP www-data
ENV APACHE_LOG_DIR /var/log/apache2

EXPOSE 80
CMD ["/usr/sbin/httpd", "-D", "FOREGROUND"]

WORKDIR /var/www/localhost/htdocs
COPY wwwroot/* /var/www/localhost/htdocs