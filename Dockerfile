FROM epitechcontent/epitest-docker

WORKDIR /app
RUN conan profile new default --detect
RUN conan remote add bincrafters https://bincrafters.jfrog.io/artifactory/api/conan/public-conan
RUN conan config set general.revisions_enabled=1
RUN pip install conan --upgrade

CMD ["make"]
