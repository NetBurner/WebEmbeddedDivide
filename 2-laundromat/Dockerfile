FROM node:22.6-alpine3.19

WORKDIR /app
COPY wwwroot/* /app

RUN npm install

EXPOSE 8000
CMD ["npm", "start"]