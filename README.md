# Bridging the Web-Embedded Divide
This repository is part of a tutorial on <a href="https://www.netburner.com">NetBurner.com</a>.

This project was developed for NetBurner's NNDK 3.x

## [Part I: Switch Counter](https://www.netburner.com/learn/web-embedded-divide-1-switch-counter/)

See folder `1-switch-counter`

Either follow the blog instructions to use with NBEclipse, or run `make -j` and `make -j load` with NNDK installed to build for your NetBurner device.
Further instructions on installing NNDK and using CLI tools are here: https://www.netburner.com/NBDocs/Developer/html/page_command_line_tools.html

For the web server, either follow the blog instructions or use standard Docker commands to build and run the web server component locally via Docker:

```
docker build -t web-embedded-blog-1 .
docker run --rm -p 80:80 --name web-embedded-blog-1 -d web-embedded-blog-1
```

Once running, you can access the web server at http://localhost or your computer's IP.

If you get ` Warning: SQLite3::query(): Unable to prepare statement: 1, no such table: device_switches in /var/www/localhost/htdocs/device.php on line 64` then use the Setup Database link to create the appropriate database table.

Set up the NetBurner by modifying the URL with the IP of your Docker host computer like so:
`http://192.168.1.101/device.php?resource=switches`

You can find your computer's IP using a terminal command like `ifconfig` (Linux and Mac) or `ipconfig` (Windows).

After clicking Update Record to save your NetBurner's configuration, flip a DIP switch or reboot the NetBurner and refresh the Docker PHP page, you should see a new entry appear.

The container and image can be removed with:
```
docker rm web-embedded-blog-1
docker rmi web-embedded-blog-1
```

Database files are saved in the `/tmp` folder of the web server, so they're temporary and may need re-setting-up after reboots. To make them more permanent, change the database file path somewhere intentional.

## Part II: TBD
