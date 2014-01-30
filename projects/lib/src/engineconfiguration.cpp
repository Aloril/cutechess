/*
    This file is part of Cute Chess.

    Cute Chess is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Cute Chess is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Cute Chess.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "engineconfiguration.h"

#include "engineoption.h"
#include "enginetextoption.h"
#include "engineoptionfactory.h"

EngineConfiguration::EngineConfiguration()
	: m_variants(QStringList() << "standard"),
	  m_whiteEvalPov(false),
	  m_validateClaims(true),
	  m_restartMode(RestartAuto),
	  m_rating(0)
{
}

EngineConfiguration::EngineConfiguration(const QString& name,
                                         const QString& command,
					 const QString& protocol)
	: m_name(name),
	  m_command(command),
	  m_protocol(protocol),
	  m_variants(QStringList() << "standard"),
	  m_whiteEvalPov(false),
	  m_validateClaims(true),
	  m_restartMode(RestartAuto),
	  m_rating(0)
{
}

EngineConfiguration::EngineConfiguration(const QVariant& variant)
	: m_variants(QStringList() << "standard"),
	  m_whiteEvalPov(false),
	  m_validateClaims(true),
	  m_restartMode(RestartAuto),
	  m_rating(0)
{
	const QVariantMap map = variant.toMap();

	setName(map["name"].toString());
	setCommand(map["command"].toString());
	setWorkingDirectory(map["workingDirectory"].toString());
	setProtocol(map["protocol"].toString());

	if (map.contains("initStrings"))
		setInitStrings(map["initStrings"].toStringList());
	if (map.contains("whitepov"))
		setWhiteEvalPov(map["whitepov"].toBool());

	if (map.contains("restart"))
	{
		const QString val(map["restart"].toString());
		if (val == "auto")
			setRestartMode(RestartAuto);
		else if (val == "on")
			setRestartMode(RestartOn);
		else if (val == "off")
			setRestartMode(RestartOff);
	}

	if (map.contains("validateClaims"))
		setClaimsValidated(map["validateClaims"].toBool());

	if (map.contains("variants"))
		setSupportedVariants(map["variants"].toStringList());

	if (map.contains("options"))
	{
		const QVariantList optionsList = map["options"].toList();
		EngineOption* option = 0;

		foreach (const QVariant& optionVariant, optionsList)
		{
			if ((option = EngineOptionFactory::create(optionVariant.toMap())) != 0)
				addOption(option);
		}
	}

	if (map.contains("rating"))
	{
		setRating(map["rating"].toInt());
	}
}

EngineConfiguration::EngineConfiguration(const EngineConfiguration& other)
	: m_name(other.m_name),
	  m_command(other.m_command),
	  m_workingDirectory(other.m_workingDirectory),
	  m_protocol(other.m_protocol),
	  m_arguments(other.m_arguments),
	  m_initStrings(other.m_initStrings),
	  m_variants(other.m_variants),
	  m_whiteEvalPov(other.m_whiteEvalPov),
	  m_validateClaims(other.m_validateClaims),
	  m_restartMode(other.m_restartMode),
	  m_rating(other.m_rating)
{
	foreach (const EngineOption* option, other.options())
		addOption(option->copy());
}

EngineConfiguration::~EngineConfiguration()
{
	qDeleteAll(m_options);
}

QVariant EngineConfiguration::toVariant() const
{
	QVariantMap map;

	map.insert("name", m_name);
	map.insert("command", m_command);
	map.insert("workingDirectory", m_workingDirectory);
	map.insert("protocol", m_protocol);

	if (!m_initStrings.isEmpty())
		map.insert("initStrings", m_initStrings);
	if (m_whiteEvalPov)
		map.insert("whitepov", true);

	if (m_restartMode == RestartOn)
		map.insert("restart", "on");
	else if (m_restartMode == RestartOff)
		map.insert("restart", "off");

	if (!m_validateClaims)
		map.insert("validateClaims", false);

	if (m_variants.count("standard") != m_variants.count())
		map.insert("variants", m_variants);

	if (!m_options.isEmpty())
	{
		QVariantList optionsList;
		foreach (const EngineOption* option, m_options)
			optionsList.append(option->toVariant());

		map.insert("options", optionsList);
	}

	if (m_rating)
		map.insert("rating", m_rating);

	return map;
}

void EngineConfiguration::setName(const QString& name)
{
	m_name = name;
}

void EngineConfiguration::setCommand(const QString& command)
{
	m_command = command;
}

void EngineConfiguration::setProtocol(const QString& protocol)
{
	m_protocol = protocol;
}

void EngineConfiguration::setRating(const int rating)
{
	m_rating = rating && rating > 0 ? rating : 0;
}

void EngineConfiguration::setWorkingDirectory(const QString& workingDir)
{
	m_workingDirectory = workingDir;
}

QString EngineConfiguration::name() const
{
	return m_name;
}

QString EngineConfiguration::command() const
{
	return m_command;
}

QString EngineConfiguration::workingDirectory() const
{
	return m_workingDirectory;
}

QString EngineConfiguration::protocol() const
{
	return m_protocol;
}

int EngineConfiguration::rating() const
{
	return m_rating;
}

QStringList EngineConfiguration::arguments() const
{
	return m_arguments;
}

void EngineConfiguration::setArguments(const QStringList& arguments)
{
	m_arguments = arguments;
}

void EngineConfiguration::addArgument(const QString& argument)
{
	m_arguments << argument;
}

QStringList EngineConfiguration::initStrings() const
{
	return m_initStrings;
}

void EngineConfiguration::setInitStrings(const QStringList& initStrings)
{
	m_initStrings = initStrings;
}

void EngineConfiguration::addInitString(const QString& initString)
{
	m_initStrings << initString.split('\n');
}

QStringList EngineConfiguration::supportedVariants() const
{
	return m_variants;
}

void EngineConfiguration::setSupportedVariants(const QStringList& variants)
{
	m_variants = variants;
}

QList<EngineOption*> EngineConfiguration::options() const
{
	return m_options;
}

void EngineConfiguration::setOptions(const QList<EngineOption*>& options)
{
	qDeleteAll(m_options);
	m_options = options;
}

void EngineConfiguration::addOption(EngineOption* option)
{
	Q_ASSERT(option != 0);

	m_options << option;
}

void EngineConfiguration::setOption(const QString& name, const QVariant& value)
{
	foreach (EngineOption* option, m_options)
	{
		if (option->name() == name)
		{
			if (!option->isValid(value))
			{
				qWarning("Invalid value for engine option %s: %s",
					 qPrintable(name),
					 qPrintable(value.toString()));
			}
			else
				option->setValue(value);

			return;
		}
	}

	m_options << new EngineTextOption(name, value, value);
}

bool EngineConfiguration::whiteEvalPov() const
{
	return m_whiteEvalPov;
}

void EngineConfiguration::setWhiteEvalPov(bool whiteEvalPov)
{
	m_whiteEvalPov = whiteEvalPov;
}

EngineConfiguration::RestartMode EngineConfiguration::restartMode() const
{
	return m_restartMode;
}

void EngineConfiguration::setRestartMode(RestartMode mode)
{
	m_restartMode = mode;
}

bool EngineConfiguration::areClaimsValidated() const
{
	return m_validateClaims;
}

void EngineConfiguration::setClaimsValidated(bool validate)
{
	m_validateClaims = validate;
}

EngineConfiguration& EngineConfiguration::operator=(const EngineConfiguration& other)
{
	if (this != &other)
	{
		m_name = other.m_name;
		m_command = other.m_command;
		m_workingDirectory = other.m_workingDirectory;
		m_protocol = other.m_protocol;
		m_arguments = other.m_arguments;
		m_initStrings = other.m_initStrings;
		m_variants = other.m_variants;
		m_whiteEvalPov = other.m_whiteEvalPov;
		m_validateClaims = other.m_validateClaims;
		m_restartMode = other.m_restartMode;
		m_rating = other.m_rating;

		qDeleteAll(m_options);
		m_options.clear();

		foreach (const EngineOption* option, other.m_options)
			m_options.append(option->copy());
	}
	return *this;
}
