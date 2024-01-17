#ifndef JSON_READER
#define JSON_READER

#include <iostream>
#include <vector>
#include <fstream>
#include <json.hpp>
#include <enum.hpp>
#include <iomanip>
#include <boost/filesystem.hpp>

namespace EMIRO
{
	typedef struct
	{
		int index;
		std::string header;
		float speed;
		WayPoint wp;
	} Target;

	class JsonIO
	{
	private:
		std::string file_path = "";
		std::vector<Target> data;
		std::vector<std::string> header_id;

	public:
		JsonIO();
		JsonIO(std::string path);
		std::vector<Target> get_data();
		void get_data(std::vector<Target> &target);
		~JsonIO();
		void operator=(std::string path);
		friend std::ostream &operator<<(std::ostream &os, Target target);
		void operator+=(const Target &target);
		void operator-=(const Target &target);
	};

	JsonIO::JsonIO() {}

	JsonIO::JsonIO(std::string path)
	{
		operator=(path);
	}

	inline std::vector<Target> JsonIO::get_data()
	{
		return data;
	}

	inline void JsonIO::get_data(std::vector<Target> &target)
	{
		target = data;
	}

	inline void JsonIO::operator=(std::string path)
	{
		header_id.clear();
		this->file_path = path;
		std::ifstream stream_reader(file_path);
		if (!stream_reader.is_open())
		{
			if (!boost::filesystem::exists(file_path))
			{
				boost::filesystem::path parent_path = boost::filesystem::path(file_path).parent_path();
				if (!boost::filesystem::exists(parent_path))
				{
					std::cout << "\033[31m\033[1mError :\033[0m Directory invalid\n";
					std::cout << "\033[31m\033[1mInfo :\033[0m Current path is " << boost::filesystem::current_path().string() << "\n";
					std::cout << "\033[31m\033[1mInfo :\033[0m Your input file path is " << parent_path.string() << "\n";
				}
				else
				{
					std::cout << "\033[31m\033[1mError :\033[0m File name invalid. Here is the list of exist files\n";
					int cnt = 1;
					for (const auto &entry : boost::filesystem::directory_iterator(parent_path))
						if (boost::filesystem::is_regular_file(entry.path()))
						{
							std::cout << cnt << ".\t: " << entry.path().filename() << '\n';
							cnt++;
						}
				}
				exit(EXIT_FAILURE);
			}

			std::cout << "\033[31m\033[1mError :\033[0m Failed Open File." << std::endl;

			// std::cerr << "Failed to Open File." << std::endl;
			// FILE *pipe = popen("pwd", "r");
			// if (!pipe)
			// 	return;
			// char buffer[128];
			// std::string result;
			// while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
			// 	result += buffer;
			// std::cout << "Current path : " << result << std::endl;
			// pclose(pipe);
			exit(EXIT_FAILURE);
		}

		Json::Value json_value;
		Json::CharReaderBuilder reader;
		JSONCPP_STRING errs;
		Json::parseFromStream(reader, stream_reader, &json_value, &errs);

		int counter = 1;
		data.clear();
		for (const auto &item : json_value)
		{
			header_id.push_back(item["header"].asString());
			data.push_back({counter,
							item["header"].asString(),
							item["speed"].asFloat(),
							{item["x"].asFloat(),
							 item["y"].asFloat(),
							 item["z"].asFloat(),
							 item["yaw"].asFloat()}});
			counter++;
		}
		stream_reader.close();
	}

	inline void JsonIO::operator+=(const Target &target)
	{
		for (const std::string &id : header_id)
			if (!id.compare(target.header))
			{
				std::cout << "\033[31m\033[1mError :\033[0m " << target.header << " already exist." << std::endl;
				return;
			}

		std::ifstream input_file(file_path);
		if (!input_file.is_open())
		{
			std::cerr << "\033[31m\033[1mFailed Open File.\033[0m" << std::endl;
			exit(EXIT_FAILURE);
		}

		std::vector<std::string> lines;
		std::string line;
		while (std::getline(input_file, line) && !input_file.eof())
			lines.push_back(line);
		input_file.close();

		Json::Value root;
		Json::StreamWriterBuilder builder;
		const std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
		std::ofstream stream_writer(file_path);
		if (!stream_writer.is_open())
		{
			std::cout << "Failed Open File." << std::endl;
			return;
		}
		if (lines.size() <= 0)
			stream_writer << "[\n";
		if (lines.size())
		{
			std::string latest_line = lines.back();
			lines.pop_back();
			for (std::string &line : lines)
				stream_writer << line << '\n';
			stream_writer << latest_line << ",\n";
		}

		root["header"] = target.header;
		root["speed"] = target.speed;
		root["x"] = target.wp.x;
		root["y"] = target.wp.y;
		root["z"] = target.wp.z;
		root["yaw"] = target.wp.yaw;
		writer->write(root, &stream_writer);
		stream_writer << "\n]";
		stream_writer.close();
	}

	inline void JsonIO::operator-=(const Target &target)
	{
	}

	inline std::ostream &operator<<(std::ostream &os, Target target)
	{
		os << std::fixed << std::setprecision(2) << "Index\t: " << target.index << "\nHeader\t: " << target.header << "\nSpeed\t: " << target.speed << "\nTarget\n\tx : " << target.wp.x << "\n\ty : " << target.wp.y << "\n\tz : " << target.wp.z << '\n'
		   << std::defaultfloat;
		return os;
	}

	JsonIO::~JsonIO() {}
}

#endif