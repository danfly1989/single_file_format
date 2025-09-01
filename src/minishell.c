/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dagwu <dagwu@student.42berlin.de>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/14 18:44:41 by dagwu             #+#    #+#             */
/*   Updated: 2025/06/14 18:44:46 by dagwu            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	main(int argc, char *argv[], char *env[])
{
	char	*line;
	t_dat	data;

	data = ft_duplicate_input_args(argc, argv, env);
	ft_set_main_signals();
	while (1)
	{
		line = readline("dandav>");
		if (line == NULL)
			break ;
		if (line && *line && !ft_strisspace(line) && ft_strcmp(line, "<<"))
			add_history(line);
		ft_check_var_assign_and_expand_line(&data, line);
		if (line)
			free(line);
		line = NULL;
	}
	ft_cleanup_data(&data);
	return (0);
}

void	ft_free_lines(t_dat *data)
{
	ft_free_string_array(data->ln);
	data->ln = NULL;
	ft_free_string_array(data->xln);
	data->xln = NULL;
}

void	ft_free_string_array(char **str_array)
{
	int	i;

	if (str_array == NULL)
		return ;
	i = 0;
	while (str_array[i] != NULL)
	{
		free(str_array[i]);
		str_array[i] = NULL;
		i++;
	}
	free(str_array);
}

void	ft_increment_shlvl(t_va **env_list)
{
	t_va	*node;
	char	*lvl_str;
	int		level;

	node = *env_list;
	while (node)
	{
		if (ft_strcmp(node->name, "SHLVL") == 0)
		{
			level = ft_atoi(node->value) + 1;
			if (level < 1)
				level = 1;
			lvl_str = ft_itoa(level);
			if (!lvl_str)
				lvl_str = ft_strdup("1");
			free(node->value);
			node->value = lvl_str;
			return ;
		}
		node = node->next;
	}
	ft_create_shlvl(env_list);
}

int	ft_create_shlvl(t_va **env_list)
{
	t_va	*new_node;

	new_node = malloc(sizeof(t_va));
	if (!new_node)
		return (0);
	new_node->name = ft_strdup("SHLVL");
	new_node->value = ft_itoa(1);
	if (!new_node->name || !new_node->value)
	{
		free(new_node->name);
		free(new_node->value);
		free(new_node);
		return (0);
	}
	new_node->next = *env_list;
	*env_list = new_node;
	return (1);
}

t_dat	ft_duplicate_input_args(int argc, char **argv, char **env)
{
	t_dat	data;

	(void)argc;
	data.av = NULL;
	data.ev = NULL;
	data.lo = NULL;
	data.ln = NULL;
	data.xln = NULL;
	data.tmp1 = NULL;
	data.tmp2 = NULL;
	data.i = 0;
	data.j = 0;
	data.k = 0;
	data.tot = 0;
	data.st = 0;
	data.last_exit_status = 0;
	data.avs = NULL;
	data.evs = NULL;
	data.qtypes = NULL;
	data.av = create_lst_frm_arr(argv + 1, NULL, 0, ft_create_node);
	data.ev = create_lst_frm_arr(env, NULL, 0, ft_create_var_node);
	ft_increment_shlvl(&data.ev);
	return (data);
}

void	ft_cleanup_data(t_dat *data)
{
	if (data->ev)
		ft_free_list(data->ev);
	if (data->av)
		ft_free_list(data->av);
	if (data->lo)
		ft_free_list(data->lo);
	if (data->ln)
		ft_free_string_array(data->ln);
	if (data->xln)
		ft_free_string_array(data->xln);
	if (data->tmp1)
		free(data->tmp1);
	if (data->tmp2)
		free(data->tmp2);
	if (data->qtypes)
		free(data->qtypes);
	if (data->evs)
		ft_free_string_array(data->evs);
	if (data->avs)
		ft_free_string_array(data->avs);
	ft_nullify_pointers(data);
}

void	ft_nullify_pointers(t_dat *data)
{
	data->ev = NULL;
	data->av = NULL;
	data->lo = NULL;
	data->ln = NULL;
	data->xln = NULL;
	data->tmp1 = NULL;
	data->tmp2 = NULL;
	data->qtypes = NULL;
	data->evs = NULL;
	data->avs = NULL;
}

void	ft_cleanup_exit(t_dat *data, int flag)
{
	ft_cleanup_data(data);
	rl_clear_history();
	exit(flag);
}

t_va	*create_lst_frm_arr(char **arr, t_va *h, int i, t_va *(*f)(char *))
{
	t_va	*current;
	t_va	*new_node;

	current = NULL;
	while (arr[i])
	{
		new_node = f(arr[i]);
		if (!new_node)
		{
			if (h)
				ft_free_list(h);
			return (NULL);
		}
		if (h == NULL)
			h = new_node;
		else
			current->next = new_node;
		current = new_node;
		i++;
	}
	return (h);
}

t_va	*ft_create_node(char *str)
{
	t_va	*node;

	node = malloc(sizeof(t_va));
	if (!node)
		return (NULL);
	node->name = NULL;
	node->value = ft_strdup(str);
	node->next = NULL;
	return (node);
}

t_va	*ft_create_var_node(char *str)
{
	t_va	*node;
	char	*equal_pos;

	if (!str)
		return (NULL);
	equal_pos = ft_strchr(str, '=');
	if (!equal_pos)
		return (NULL);
	node = malloc(sizeof(t_va));
	if (!node)
		return (NULL);
	node->name = ft_extract_var_name(str);
	node->value = ft_extract_var_value(equal_pos + 1, 0, 0);
	node->next = NULL;
	if (!node->name || !node->value)
	{
		free(node->name);
		free(node->value);
		free(node);
		return (NULL);
	}
	return (node);
}

char	*ft_extract_var_name(char *str)
{
	char	*name;
	size_t	i;
	size_t	len;

	len = 0;
	while (str[len] && str[len] != '=')
		len++;
	name = malloc(len + 1);
	if (!name)
		return (NULL);
	i = 0;
	while (i < len)
	{
		name[i] = str[i];
		i++;
	}
	name[i] = '\0';
	return (name);
}

char	*ft_extract_var_value(char *str, char quote, size_t len)
{
	char	*val;
	size_t	i;

	if (!str || str[0] == '\0')
		return (NULL);
	if (str[0] == '"' || str[0] == '\'')
	{
		quote = str[0];
		str++;
	}
	while (str[len] && str[len] != quote)
		len++;
	val = malloc(len + 1);
	if (!val)
		return (NULL);
	i = -1;
	while (++i < len)
		val[i] = str[i];
	val[len] = '\0';
	return (val);
}

void	ft_free_list(t_va *head)
{
	t_va	*tmp;

	while (head != NULL)
	{
		tmp = head;
		head = head->next;
		if (tmp->name)
			free(tmp->name);
		if (tmp->value)
			free(tmp->value);
		free(tmp);
	}
}

int	ft_strisspace(char *str)
{
	int	i;

	if (!str)
		return (0);
	i = 0;
	while (str[i] && str[i] != '\0')
	{
		if (!ft_isspace(str[i]))
			return (0);
		i++;
	}
	return (1);
}

void	ft_nested_sigint_handler(int sig)
{
	(void)sig;
	rl_replace_line("", 0);
	rl_redisplay();
}

void	ft_parent_sigint_handler(int sig)
{
	(void)sig;
	write(1, "\n", 1);
	rl_replace_line("", 0);
	rl_on_new_line();
	rl_redisplay();
}

void	ft_child_sigint_handler(int sig)
{
	if (sig == SIGINT)
	{
		write(1, "\n", 1);
	}
	else if (sig == SIGQUIT)
	{
		write(1, "Quit (core dumped)\n", 19);
	}
}

void	ft_heredoc_sigint_handler(int sig)
{
	if (sig == SIGINT)
	{
		write(1, "\n", 1);
		exit(130);
	}
}

void	ft_set_main_signals(void)
{
	struct sigaction	sa;

	sa.sa_handler = ft_parent_sigint_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sigaction(SIGINT, &sa, NULL);
	signal(SIGQUIT, SIG_IGN);
}

void	ft_set_child_signals(void)
{
	struct sigaction	sa;

	sa.sa_handler = ft_child_sigint_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);
	signal(SIGQUIT, SIG_DFL);
}

void	ft_set_main_nested_signals(void)
{
	struct sigaction	sa;

	sa.sa_handler = ft_nested_sigint_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sigaction(SIGINT, &sa, NULL);
	signal(SIGQUIT, SIG_IGN);
}

void	ft_set_no_pipe_child_signals(t_dat *d)
{
	if (ft_strcmp(d->xln[0], "./minishell") == 0)
		ft_set_main_nested_signals();
	else
		ft_set_child_signals();
}

void	ft_set_heredoc_signals(void)
{
	struct sigaction	sa;

	sa.sa_handler = ft_heredoc_sigint_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);
	signal(SIGQUIT, SIG_DFL);
}

int	ft_skip_quote(char *str, int i)
{
	char	quote;

	quote = str[i++];
	while (str[i] && str[i] != quote)
		i++;
	if (str[i])
		i++;
	return (i);
}

int	ft_skip_token(char *str, int i)
{
	while (str[i] && str[i] != ' ')
	{
		if (str[i] == '\'' || str[i] == '"')
			i = ft_skip_quote(str, i);
		else
			i++;
	}
	return (i);
}

int	ft_count_tokens(char *str)
{
	int	i;
	int	count;

	i = 0;
	count = 0;
	while (str[i])
	{
		while (str[i] == ' ')
			i++;
		if (!str[i])
			break ;
		count++;
		if (str[i] == '\'' || str[i] == '"')
			i = ft_skip_quote(str, i);
		else
			i = ft_skip_token(str, i);
	}
	return (count);
}

int	ft_get_token_end(char *str, int i)
{
	if (str[i] == '\'' || str[i] == '"')
		return (ft_skip_quote(str, i));
	return (ft_skip_token(str, i));
}

void	ft_detect_quote_type(char *token, int *quote_type)
{
	char	*eq;
	char	quote;

	if (token[0] == '\'')
		*quote_type = 1;
	else if (token[0] == '"')
		*quote_type = 2;
	else
	{
		eq = ft_strchr(token, '=');
		if (eq && (*(eq + 1) == '\'' || *(eq + 1) == '"'))
		{
			quote = *(eq + 1);
			if (quote == '\'')
				*quote_type = 1;
			else
				*quote_type = 2;
		}
	}
}

char	*ft_extract_token(char *str, t_dat *d, int *quote_type)
{
	int		start;
	int		end;
	char	*token;

	start = d->i;
	*quote_type = 0;
	end = ft_get_token_end(str, d->i);
	d->i = end;
	token = ft_strndup(str + start, end - start);
	if (!token)
		return (NULL);
	ft_detect_quote_type(token, quote_type);
	return (token);
}

void	ft_reset_iterators(t_dat *data)
{
	data->i = 0;
	data->j = 0;
	data->k = 0;
	data->tot = 0;
	data->st = 0;
}

char	**ft_free_token_quote(char **tokens, int *quote_types)
{
	if (tokens)
		free(tokens);
	if (quote_types)
		free(quote_types);
	return (NULL);
}

char	**ft_tokenize_line(t_dat *d, char *str, int **quote_types_out)
{
	char	**tokens;

	ft_reset_iterators(d);
	d->k = ft_count_tokens(str);
	tokens = malloc(sizeof(char *) * (d->k + 1));
	d->qtypes = malloc(sizeof(int) * (d->k + 1));
	if (!tokens || !d->qtypes)
		return (ft_free_token_quote(tokens, d->qtypes));
	while (str[d->i])
	{
		while (str[d->i] == ' ')
			d->i++;
		if (!str[d->i])
			break ;
		tokens[d->j] = ft_extract_token(str, d, &d->qtypes[d->j]);
		d->j++;
	}
	tokens[d->j] = NULL;
	d->qtypes[d->j] = -1;
	*quote_types_out = d->qtypes;
	return (tokens);
}

char	*ft_get_var_value(t_va *list, const char *name)
{
	size_t	n;

	n = ft_strlen(name);
	while (list)
	{
		if (list->name && ft_strncmp(list->name, name, n) == 0)
			return (list->value);
		list = list->next;
	}
	return (NULL);
}

char	*ft_extract_var_key(const char *str, size_t *i)
{
	size_t	start;
	char	*key;

	start = *i;
	if (str[*i] == '?')
	{
		(*i)++;
		return (ft_strdup("?"));
	}
	while (str[*i] && (ft_isalnum(str[*i]) || str[*i] == '_'))
		(*i)++;
	key = ft_substr(str, start, *i - start);
	return (key);
}

char	*ft_strjoin_char(const char *s, char c)
{
	char	*new;
	size_t	len;

	if (!s)
		return (NULL);
	len = ft_strlen(s);
	new = malloc(len + 2);
	if (!new)
		return (NULL);
	ft_strlcpy(new, s, len + 1);
	new[len] = c;
	new[len + 1] = '\0';
	return (new);
}

void	ft_expand_loop(char *token, t_dat *data, char **res, size_t *i)
{
	char	*key;
	char	*val;
	char	*tmp;

	key = ft_extract_var_key(token, i);
	if (!key)
		return ;
	val = NULL;
	if (data->lo != NULL)
		val = ft_get_var_value(data->lo, key);
	if (val == NULL)
		val = ft_get_var_value(data->ev, key);
	if (val != NULL)
	{
		tmp = *res;
		*res = ft_strjoin(*res, val);
		free(tmp);
	}
	free(key);
}

char	*ft_expand_token(char *token, t_dat *data, int qt, size_t i)
{
	char	*res;
	char	*tmp;

	if (qt == 1)
		return (ft_strdup(token));
	res = ft_calloc(1, sizeof(char));
	while (token[i])
	{
		if (token[i] == '$' && token[i + 1] && (ft_isalpha(token[i + 1])
				|| token[i + 1] == '_' || token[i + 1] == '?'))
		{
			i++;
			ft_expand_loop(token, data, &res, &i);
		}
		else
		{
			tmp = res;
			res = ft_strjoin_char(res, token[i++]);
			free(tmp);
		}
	}
	return (res);
}

void	*ft_free_error_expanded(char **expanded, int i)
{
	while (--i >= 0)
		free(expanded[i]);
	free(expanded);
	return (NULL);
}

char	**ft_expand_tokens(t_dat *d, char **tokens, int *qtypes, int i)
{
	char	**expanded;

	while (tokens[i])
		i++;
	expanded = malloc(sizeof(char *) * (i + 1));
	if (!expanded)
		return (NULL);
	i = 0;
	while (tokens[i])
	{
		d->tmp2 = ft_expand_exit_status(d, tokens[i]);
		expanded[i] = ft_expand_token(d->tmp2, d, qtypes[i], 0);
		free(d->tmp2);
		d->tmp2 = NULL;
		if (!expanded[i])
			return (ft_free_error_expanded(expanded, i));
		i++;
	}
	expanded[i] = NULL;
	return (expanded);
}

void	ft_strip_surrounding_quotes(char *s)
{
	size_t	len;
	size_t	j;

	len = ft_strlen(s);
	if (len >= 2 && ((s[0] == '"' && s[len - 1] != '"') || (s[0] == '\''
				&& s[len - 1] != '\'')))
		return ;
	if (len >= 2 && ((s[0] == '"' && s[len - 1] == '"') || (s[0] == '\''
				&& s[len - 1] == '\'')))
	{
		j = 1;
		while (j < len - 1)
		{
			s[j - 1] = s[j];
			j++;
		}
		s[j - 1] = '\0';
	}
}

void	ft_strip_quotes_after_equal(char *s)
{
	char	*eq;
	char	quote;
	size_t	j;
	size_t	len;

	eq = ft_strchr(s, '=');
	len = ft_strlen(s);
	if (eq && ((eq[1] == '"' && s[len - 1] != '"') || (eq[1] == '\'' && s[len
				- 1] != '\'')))
		return ;
	if (eq && ((eq[1] == '"' && s[len - 1] == '"') || (eq[1] == '\'' && s[len
				- 1] == '\'')))
	{
		quote = eq[1];
		j = 0;
		while (eq[2 + j] && eq[2 + j] != quote)
		{
			eq[1 + j] = eq[2 + j];
			j++;
		}
		eq[1 + j] = '\0';
	}
}

void	ft_strip_quotes_from_xln(t_dat *d)
{
	size_t	i;

	i = 0;
	if (!d || !d->xln)
		return ;
	while (d->xln[i])
	{
		if (d->xln[i])
		{
			ft_strip_surrounding_quotes(d->xln[i]);
			ft_strip_quotes_after_equal(d->xln[i]);
		}
		i++;
	}
}

int	ft_valid_var(char *str)
{
	size_t	i;

	if (!str || (!ft_isalpha(str[0]) && str[0] != '_'))
		return (0);
	i = 1;
	while (str[i] && str[i] != '=')
	{
		if (!ft_isalnum(str[i]) && str[i] != '_')
			return (0);
		i++;
	}
	if (str[i] != '=')
		return (0);
	return (1);
}

t_va	*ft_find_var(t_va *list, const char *name)
{
	while (list)
	{
		if (list->name && ft_strncmp(list->name, name, ft_strlen(name)) == 0)
			return (list);
		list = list->next;
	}
	return (NULL);
}

int	ft_update_var_value(t_va *node, const char *value)
{
	char	*new_value;

	new_value = ft_strdup(value);
	if (!new_value)
		return (0);
	free(node->value);
	node->value = new_value;
	return (1);
}

int	ft_add_local_var(t_dat *data, char *str)
{
	t_va	*new_node;
	t_va	*last;

	new_node = ft_create_var_node(str);
	if (!new_node)
		return (0);
	if (!data->lo)
	{
		data->lo = new_node;
		return (1);
	}
	last = data->lo;
	while (last->next)
		last = last->next;
	last->next = new_node;
	return (1);
}

void	ft_update_local_variables(t_dat *d)
{
	int		i;
	char	*name;
	t_va	*node;

	if (!d || !d->xln)
		return ;
	i = 0;
	while (d->xln[i])
	{
		name = ft_extract_var_name(d->xln[i]);
		if (!name)
			return ;
		node = ft_find_var(d->lo, name);
		if (node)
			ft_update_var_value(node, ft_strchr(d->xln[i], '=') + 1);
		else
			ft_add_local_var(d, d->xln[i]);
		free(name);
		i++;
	}
}

int	ft_all_valid_lvar(t_dat *data, char **arr)
{
	int	i;

	if (!arr)
		return (0);
	ft_reset_iterators(data);
	i = 0;
	while (arr[i])
		i++;
	data->tot = i;
	i = 0;
	while (arr[i])
	{
		if (!ft_valid_var(arr[i]))
		{
			data->st = i;
			return (0);
		}
		i++;
	}
	return (1);
}

int	ft_is_number(const char *str)
{
	size_t	i;

	if (str == NULL || *str == '\0')
		return (0);
	i = 0;
	if (str[i] == '+' || str[i] == '-')
		i++;
	while (str[i] != '\0')
	{
		if (ft_isdigit(str[i]) == 0)
			return (0);
		i++;
	}
	return (1);
}

char	*ft_get_val_from_list(t_va *head, const char *key)
{
	t_va	*cur;
	size_t	len;

	if (!head || !key)
		return (NULL);
	len = ft_strlen(key);
	cur = head;
	while (cur)
	{
		if (ft_strncmp(cur->name, key, len + 1) == 0)
			return (cur->value);
		cur = cur->next;
	}
	return (NULL);
}

int	ft_update_existing_var(t_va *node, const char *name, const char *val)
{
	while (node)
	{
		if (ft_strncmp(node->name, name, ft_strlen(name)) == 0)
		{
			free(node->value);
			node->value = ft_strdup(val);
			if (!node->value)
				perror("minishell: malloc error");
			return (1);
		}
		node = node->next;
	}
	return (0);
}

void	ft_create_env_variable(t_dat *d, const char *name, const char *value)
{
	t_va	*new_node;

	new_node = malloc(sizeof(t_va));
	if (!new_node)
		return (perror("minishell: malloc error"));
	new_node->name = ft_strdup(name);
	new_node->value = ft_strdup(value);
	new_node->next = d->ev;
	d->ev = new_node;
	if (!new_node->name || !new_node->value)
	{
		free(new_node->name);
		free(new_node->value);
		free(new_node);
		perror("minishell: malloc error");
	}
}

void	ft_update_env_variable(t_dat *d, const char *name, const char *value)
{
	if (!ft_update_existing_var(d->ev, name, value))
		ft_create_env_variable(d, name, value);
}

void	ft_pwd(void)
{
	char	*cwd;

	cwd = getcwd(NULL, 0);
	if (cwd == NULL)
	{
		perror("pwd error");
		return ;
	}
	printf("%s\n", cwd);
	free(cwd);
}

void	ft_update_directories(t_dat *data, char *oldpwd)
{
	char	*newpwd;

	newpwd = getcwd(NULL, 0);
	ft_update_env_variable(data, "OLDPWD", oldpwd);
	ft_update_env_variable(data, "PWD", newpwd);
	free(oldpwd);
	free(newpwd);
}

void	ft_cd_error(char *path)
{
	char	*msg;

	msg = strerror(errno);
	write(2, "minishell: cd: ", 15);
	write(2, path, ft_strlen(path));
	write(2, ": ", 2);
	write(2, msg, ft_strlen(msg));
	write(2, "\n", 1);
}

void	ft_change_directory(t_dat *data, size_t k)
{
	char	*path;
	char	*oldpwd;

	oldpwd = getcwd(NULL, 0);
	if (data->xln[k + 1] == NULL || ft_strcmp(data->xln[k + 1], "~") == 0)
	{
		path = ft_get_val_from_list(data->ev, "HOME");
		if (path == NULL)
		{
			write(2, "cd: HOME not set\n", 17);
			return ;
		}
	}
	else
		path = data->xln[k + 1];
	if (chdir(path) == 0)
		ft_update_directories(data, oldpwd);
	else
		ft_cd_error(path);
}

void	ft_echo(char **arr, size_t k)
{
	int	i;
	int	newline;

	i = 1;
	newline = 1;
	while (arr[k + i] != NULL && ft_strncmp(arr[k + i], "-n", 2) == 0)
	{
		newline = 0;
		i++;
	}
	while (arr[k + i] != NULL)
	{
		printf("%s", arr[k + i]);
		i++;
		if (arr[k + i] != NULL)
			printf(" ");
	}
	if (newline)
		printf("\n");
}

void	ft_exit_numeric_error(char *arg)
{
	write(2, "minishell: exit: ", 18);
	write(2, arg, ft_strlen(arg));
	write(2, ": numeric argument required\n", 29);
}

void	ft_exit(t_dat *data, size_t k)
{
	int	status;

	if (data->xln[k + 1] && data->xln[k + 2])
	{
		write(2, "minishell: exit: too many arguments\n", 36);
		return ;
	}
	rl_clear_history();
	if (data->xln[k + 1] == NULL)
		ft_cleanup_exit(data, 0);
	if (ft_is_number(data->xln[k + 1]) == 0)
	{
		ft_exit_numeric_error(data->xln[k + 1]);
		ft_cleanup_exit(data, 255);
	}
	status = ft_atoi(data->xln[k + 1]);
	ft_cleanup_exit(data, status % 256);
}

void	ft_env(t_dat *data)
{
	t_va	*cur;

	cur = data->ev;
	while (cur != NULL)
	{
		printf("%s=%s\n", cur->name, cur->value);
		cur = cur->next;
	}
}

void	ft_unset_multi_var(t_dat *d, size_t k)
{
	int	i;

	i = 1;
	while (d->xln[k + i] != NULL)
	{
		d->ev = ft_remove_variable_node(d->xln[k + i], d->ev, NULL);
		d->lo = ft_remove_variable_node(d->xln[k + i], d->lo, NULL);
		i++;
	}
}

t_va	*ft_remove_variable_node(const char *key, t_va *head, t_va *prev)
{
	t_va	*cur;
	t_va	*next;

	cur = head;
	while (cur != NULL)
	{
		if (ft_strncmp(cur->name, key, ft_strlen(key)) == 0)
		{
			next = cur->next;
			free(cur->name);
			free(cur->value);
			free(cur);
			if (prev == NULL)
				head = next;
			else
				prev->next = next;
			break ;
		}
		prev = cur;
		cur = cur->next;
	}
	return (head);
}

int	ft_var_name_only(char *str)
{
	size_t	i;

	if (!str)
		return (0);
	if (!ft_isalpha(str[0]) && str[0] != '_')
		return (0);
	i = 1;
	while (str[i])
	{
		if (!ft_isalnum(str[i]) && str[i] != '_')
			return (0);
		i++;
	}
	return (1);
}

void	ft_append_env_var(t_dat *data, char *key, char *value)
{
	t_va	*new;
	t_va	*cur;

	new = malloc(sizeof(t_va));
	if (!new)
		return ;
	new->name = ft_strdup(key);
	new->value = ft_strdup(value);
	new->next = NULL;
	cur = data->ev;
	if (!cur)
	{
		data->ev = new;
		return ;
	}
	while (cur->next)
		cur = cur->next;
	cur->next = new;
}

void	ft_export_type2(t_dat *data, char *str)
{
	char	*name;
	char	*val;
	t_va	*cur;

	val = NULL;
	name = str;
	cur = data->lo;
	while (cur)
	{
		if (ft_strncmp(cur->name, name, ft_strlen(name)) == 0)
		{
			val = cur->value;
			break ;
		}
		cur = cur->next;
	}
	if (val)
		ft_append_env_var(data, name, val);
}

t_va	*ft_export_type1_ext(char *name, char *val)
{
	t_va	*new;

	new = malloc(sizeof(t_va));
	new->name = name;
	new->value = ft_strdup(val);
	new->next = NULL;
	return (new);
}

void	ft_export_type1(t_va **head, char *s, char *name, char *val)
{
	t_va	*cur;
	t_va	*new;
	t_va	*prev;

	name = ft_extract_var_name(s);
	val = ft_strchr(s, '=') + 1;
	cur = *head;
	while (cur)
	{
		if (ft_strcmp(cur->name, name) == 0)
		{
			free(cur->value);
			cur->value = ft_strdup(val);
			free(name);
			return ;
		}
		prev = cur;
		cur = cur->next;
	}
	new = ft_export_type1_ext(name, val);
	if (prev)
		prev->next = new;
	else
		*head = new;
}

void	ft_print_export(t_va *head)
{
	t_va	*cur;

	cur = head;
	while (cur != NULL)
	{
		printf("declare -x %s=\"%s\"\n", cur->name, cur->value);
		cur = cur->next;
	}
}

t_va	*ft_merge_sorted_lists(t_va *a, t_va *b)
{
	t_va	*result;

	if (a == NULL)
		return (b);
	else if (b == NULL)
		return (a);
	if (ft_strcmp(a->name, b->name) <= 0)
	{
		result = a;
		result->next = ft_merge_sorted_lists(a->next, b);
	}
	else
	{
		result = b;
		result->next = ft_merge_sorted_lists(a, b->next);
	}
	return (result);
}

void	ft_split_list(t_va *source, t_va **front, t_va **back)
{
	t_va	*fast;
	t_va	*slow;

	slow = source;
	fast = source->next;
	while (fast != NULL)
	{
		fast = fast->next;
		if (fast != NULL)
		{
			slow = slow->next;
			fast = fast->next;
		}
	}
	*front = source;
	*back = slow->next;
	slow->next = NULL;
}

void	ft_sort_list_by_name(t_va **head_ref)
{
	t_va	*head;
	t_va	*a;
	t_va	*b;

	if (*head_ref == NULL || (*head_ref)->next == NULL)
		return ;
	head = *head_ref;
	ft_split_list(head, &a, &b);
	ft_sort_list_by_name(&a);
	ft_sort_list_by_name(&b);
	*head_ref = ft_merge_sorted_lists(a, b);
}

t_va	*ft_duplicate_node(const t_va *node)
{
	t_va	*new;

	new = malloc(sizeof(t_va));
	if (new == NULL)
		return (NULL);
	new->name = ft_strdup(node->name);
	new->value = ft_strdup(node->value);
	new->next = NULL;
	if (new->name == NULL || new->value == NULL)
	{
		free(new->name);
		free(new->value);
		free(new);
		return (NULL);
	}
	return (new);
}

int	ft_append_dup_node(const t_va *cur, t_va **head, t_va **tail)
{
	t_va	*new_node;

	new_node = ft_duplicate_node(cur);
	if (new_node == NULL)
	{
		ft_free_list(*head);
		return (0);
	}
	if (*tail == NULL)
	{
		*head = new_node;
		*tail = new_node;
	}
	else
	{
		(*tail)->next = new_node;
		*tail = new_node;
	}
	return (1);
}

t_va	*ft_duplicate_list(const t_va *head)
{
	const t_va	*cur;
	t_va		*new_head;
	t_va		*new_tail;

	cur = head;
	new_head = NULL;
	new_tail = NULL;
	while (cur != NULL)
	{
		if (!ft_append_dup_node(cur, &new_head, &new_tail))
			return (NULL);
		cur = cur->next;
	}
	return (new_head);
}

void	ft_print_sorted_env(t_va *head)
{
	t_va	*sorted;

	sorted = ft_duplicate_list(head);
	ft_sort_list_by_name(&sorted);
	ft_print_export(sorted);
	ft_free_list(sorted);
}

void	ft_export_error(char *arg, char *message)
{
	write(2, "export: '", 9);
	write(2, arg, ft_strlen(arg));
	write(2, "': ", 3);
	write(2, message, ft_strlen(message));
	write(2, "\n", 1);
}

void	ft_export_multi_var(t_dat *data, size_t k)
{
	char	*message;
	int		i;

	message = "not a valid identifier";
	if (data->xln[k + 1] == NULL)
	{
		ft_print_sorted_env(data->ev);
		return ;
	}
	i = 1;
	while (data->xln[k + i] != NULL)
	{
		if (ft_valid_var(data->xln[k + i]) == 1)
		{
			ft_export_type1(&data->ev, data->xln[k + i], NULL, NULL);
			ft_add_local_var(data, data->xln[k + i]);
		}
		else if (ft_var_name_only(data->xln[k + i]) == 1)
			ft_export_type2(data, data->xln[k + i]);
		else
			ft_export_error(data->xln[k + i], message);
		i++;
	}
}

int	ft_handle_builtin(t_dat *data, size_t k)
{
	if (data == NULL || data->xln == NULL)
		return (0);
	if (ft_strcmp(data->xln[k], "pwd") == 0)
		ft_pwd();
	else if (ft_strcmp(data->xln[k], "cd") == 0)
		ft_change_directory(data, k);
	else if (ft_strcmp(data->xln[k], "echo") == 0)
		ft_echo(data->xln, k);
	else if (ft_strcmp(data->xln[k], "exit") == 0)
		ft_exit(data, k);
	else if (ft_strcmp(data->xln[k], "env") == 0)
		ft_env(data);
	else if (ft_strcmp(data->xln[k], "unset") == 0)
		ft_unset_multi_var(data, k);
	else if (ft_strcmp(data->xln[k], "export") == 0)
		ft_export_multi_var(data, k);
	else
		return (0);
	return (1);
}

void	ft_check_var_assign_and_expand_line_ext(t_dat *data, char *line)
{
	ft_strip_quotes_from_xln(data);
	if (ft_all_valid_lvar(data, data->xln))
		ft_update_local_variables(data);
	ft_external_functions(data, line);
	if (data->qtypes)
	{
		free(data->qtypes);
		data->qtypes = NULL;
	}
	ft_free_string_array(data->ln);
	data->ln = NULL;
	ft_free_string_array(data->xln);
	data->xln = NULL;
}

void	ft_check_var_assign_and_expand_line(t_dat *data, char *line)
{
	if (!data || !line)
		return ;
	data->qtypes = NULL;
	data->ln = ft_tokenize_line(data, line, &data->qtypes);
	if (!data->ln)
	{
		if (data->qtypes)
			free(data->qtypes);
		return ;
	}
	data->xln = ft_expand_tokens(data, data->ln, data->qtypes, 0);
	if (!data->xln)
	{
		if (data->qtypes)
			free(data->qtypes);
		ft_free_string_array(data->ln);
		data->ln = NULL;
		return ;
	}
	ft_check_var_assign_and_expand_line_ext(data, line);
}

int	ft_count_list(t_va *head)
{
	t_va	*cur;
	int		count;

	cur = head;
	count = 0;
	while (cur)
	{
		if (cur->name)
			count++;
		cur = cur->next;
	}
	return (count);
}

void	ft_list_to_env_array(t_dat *data)
{
	int		i;
	int		count;
	t_va	*cur;

	i = 0;
	data->tmp1 = NULL;
	count = ft_count_list(data->ev);
	data->evs = malloc((count + 1) * sizeof(char *));
	if (!data->evs)
		return ;
	cur = data->ev;
	while (cur && i < count)
	{
		data->tmp1 = ft_strjoin(cur->name, "=");
		data->evs[i] = ft_strjoin(data->tmp1, cur->value);
		free(data->tmp1);
		data->tmp1 = NULL;
		cur = cur->next;
		i++;
	}
	data->evs[i] = NULL;
}

char	*ft_join_path(char *str1, char *cmd)
{
	char	*temp;
	char	*full_path;

	temp = ft_strjoin(str1, "/");
	full_path = ft_strjoin(temp, cmd);
	free(temp);
	temp = NULL;
	return (full_path);
}

char	*ft_get_cmd_path_nested(const char *cmd)
{
	if (access(cmd, X_OK) == 0)
		return (ft_strdup(cmd));
	return (NULL);
}

char	*ft_get_cmd_path(t_dat *d, const char *cmd, int i)
{
	char	*full;

	if (!cmd || *cmd == '\0')
		return (NULL);
	if (cmd[0] == '/' || (cmd[0] == '.' && cmd[1] == '/'))
		return (ft_get_cmd_path_nested(cmd));
	d->tmp1 = ft_get_val_from_list(d->ev, "PATH");
	if (!d->tmp1)
		return (NULL);
	d->avs = ft_split(d->tmp1, ':');
	while (d->avs && d->avs[i])
	{
		full = ft_join_path(d->avs[i], (char *)cmd);
		if (access(full, X_OK) == 0)
		{
			ft_free_string_array(d->avs);
			return (full);
		}
		free(full);
		i++;
	}
	ft_free_string_array(d->avs);
	return (NULL);
}

int	ft_count_pipes(char **tokens)
{
	int	count;
	int	i;

	count = 0;
	i = 0;
	while (tokens[i])
	{
		if (ft_strcmp(tokens[i], "|") == 0)
			count++;
		i++;
	}
	return (count);
}

void	ft_cmd_not_found(char *cmd)
{
	char	*prefix;
	char	*suffix;

	prefix = "minishell: ";
	suffix = ": command not found\n";
	write(2, prefix, ft_strlen(prefix));
	write(2, cmd, ft_strlen(cmd));
	write(2, suffix, ft_strlen(suffix));
	exit(127);
}

void	ft_get_exit_stat(t_dat *d, pid_t pid)
{
	int	status;

	status = 0;
	waitpid(pid, &status, 0);
	if (WIFEXITED(status))
		d->last_exit_status = WEXITSTATUS(status);
}

void	ft_ex_single_cmd_child(t_dat *d, char *cmd_path, int saved_stdin)
{
	close(saved_stdin);
	cmd_path = ft_get_cmd_path(d, d->xln[0], 0);
	if (!cmd_path)
	{
		ft_cmd_not_found(d->xln[0]);
		exit(1);
	}
	execve(cmd_path, d->xln, d->evs);
	exit(1);
}

void	ft_ex_single_cmd_parent(t_dat *d, pid_t pid, int saved_stdin)
{
	ft_get_exit_stat(d, pid);
	if (dup2(saved_stdin, STDIN_FILENO) < 0)
		perror("dup2");
	close(saved_stdin);
}

int	ft_ex_single_cmd_setup(t_dat *d, t_rdr *r, int *saved_stdin)
{
	*saved_stdin = dup(STDIN_FILENO);
	if (*saved_stdin < 0)
	{
		perror("dup");
		return (0);
	}
	ft_set_no_pipe_child_signals(d);
	ft_parse_redirection(d->xln, r);
	if (!ft_apply_sing_redirections(r, d->xln))
	{
		close(*saved_stdin);
		return (0);
	}
	return (1);
}

void	ft_ex_single_cmd(t_dat *d, char *cmd_path)
{
	t_rdr	r;
	pid_t	pid;
	int		saved_stdin;

	if (ft_handle_builtin(d, d->st))
		return ;
	if (!ft_ex_single_cmd_setup(d, &r, &saved_stdin))
		return ;
	pid = fork();
	if (pid == 0)
		ft_ex_single_cmd_child(d, cmd_path, saved_stdin);
	else if (pid > 0)
		ft_ex_single_cmd_parent(d, pid, saved_stdin);
	else
		perror("fork");
	ft_set_main_signals();
}

void	ft_cmd_error(t_dat *data, char *line)
{
	(void)line;
	ft_free_string_array(data->evs);
	data->evs = NULL;
	return ;
}

void	ft_external_functions(t_dat *data, char *line)
{
	char	***cmd;
	int		n;

	(void)line;
	if (!data || !data->xln || !data->xln[0])
		return ;
	if (!ft_validate_syntax(data->xln))
		return ;
	ft_list_to_env_array(data);
	n = ft_count_pipes(data->xln);
	if (n > 0)
	{
		cmd = ft_parse_cmd(data, 0, 0, 0);
		if (!cmd)
			return (ft_external_functions(data, line));
		ft_execute_pipeline(data, cmd);
		ft_clean_cmd(cmd);
	}
	else
		ft_ex_single_cmd(data, NULL);
	ft_free_string_array(data->evs);
	data->evs = NULL;
}

char	**ft_extract_tokens(t_dat *data, int start, int end)
{
	char	**tokens;
	int		i;

	tokens = malloc((end - start + 1) * sizeof(char *));
	if (!tokens)
		return (NULL);
	i = 0;
	while (start < end)
	{
		tokens[i] = ft_strdup(data->xln[start]);
		if (!tokens[i])
		{
			ft_free_string_array(tokens);
			return (NULL);
		}
		start++;
		i++;
	}
	tokens[i] = NULL;
	return (tokens);
}

char	***ft_clean_cmd(char ***cmd)
{
	int	i;

	if (!cmd)
		return (NULL);
	i = 0;
	while (cmd[i])
	{
		ft_free_string_array(cmd[i]);
		i++;
	}
	free(cmd);
	return (NULL);
}

int	ft_parse_cmd_helper(t_dat *d, char ***cmd, int *idx, int *st_i)
{
	int	i;

	i = st_i[1];
	if (i < st_i[0])
		return (0);
	if (!ft_validate_segment(d->xln, st_i[0], i))
		return (0);
	cmd[*idx] = ft_extract_tokens(d, st_i[0], i);
	if (!cmd[*idx])
		return (0);
	(*idx)++;
	st_i[0] = i + 1;
	return (1);
}

char	***ft_parse_cmd(t_dat *d, int st, int i, int idx)
{
	char	***cmd;
	int		st_i[2];

	d->k = ft_count_pipes(d->xln) + 1;
	cmd = ft_calloc(d->k + 1, sizeof(char **));
	if (!cmd)
		return (NULL);
	st_i[0] = st;
	while (1)
	{
		st_i[1] = i;
		if (!d->xln[i] || !ft_strcmp(d->xln[i], "|"))
		{
			if (!ft_parse_cmd_helper(d, cmd, &idx, st_i))
				return (ft_clean_cmd(cmd));
			if (!d->xln[i])
				break ;
		}
		i++;
	}
	cmd[idx] = NULL;
	d->tot = idx;
	return (cmd);
}

void	ft_free_fd(int **fd)
{
	int	i;

	if (!fd)
		return ;
	i = 0;
	while (fd[i])
	{
		free(fd[i]);
		i++;
	}
	free(fd);
}

int	**init_fd_array(int tot)
{
	int	**fd;
	int	i;

	fd = malloc(sizeof(int *) * tot);
	if (!fd)
		return (NULL);
	i = 0;
	while (i < tot - 1)
	{
		fd[i] = malloc(sizeof(int) * 2);
		if (!fd[i])
		{
			while (--i >= 0)
				free(fd[i]);
			free(fd);
			return (NULL);
		}
		i++;
	}
	fd[tot - 1] = NULL;
	return (fd);
}

int	ft_create_pipes(int **fd, int tot)
{
	int	i;

	i = 0;
	while (i < tot - 1)
	{
		if (pipe(fd[i]) == -1)
		{
			perror("pipe");
			while (i-- > 0)
				free(fd[i]);
			free(fd);
			return (0);
		}
		i++;
	}
	return (1);
}

void	ft_setup_io(int **fd, size_t i, size_t total)
{
	if (i > 0)
		dup2(fd[i - 1][0], STDIN_FILENO);
	if (i < total - 1)
		dup2(fd[i][1], STDOUT_FILENO);
}

int	ft_is_builtin(char *cmd)
{
	if (!ft_strcmp(cmd, "pwd"))
		return (1);
	if (!ft_strcmp(cmd, "cd"))
		return (1);
	if (!ft_strcmp(cmd, "echo"))
		return (1);
	if (!ft_strcmp(cmd, "exit"))
		return (1);
	if (!ft_strcmp(cmd, "export"))
		return (1);
	if (!ft_strcmp(cmd, "unset"))
		return (1);
	if (!ft_strcmp(cmd, "env"))
		return (1);
	return (0);
}

void	ft_execute_builtin_in_child(t_dat *d, char **cmd)
{
	if (!ft_strcmp(cmd[0], "pwd"))
		ft_pwd();
	else if (!ft_strcmp(cmd[0], "cd"))
		ft_change_directory(d, 0);
	else if (!ft_strcmp(cmd[0], "echo"))
		ft_echo(cmd, 0);
	else if (!ft_strcmp(cmd[0], "exit"))
		ft_exit(d, 0);
	else if (!ft_strcmp(cmd[0], "env"))
		ft_env(d);
	else if (!ft_strcmp(cmd[0], "unset"))
		ft_unset_multi_var(d, 0);
	else if (!ft_strcmp(cmd[0], "export"))
		ft_export_multi_var(d, 0);
}

void	ft_exec_command(t_dat *d, char **cmd)
{
	char	*cmd_path;

	if (!cmd || !cmd[0])
		exit(127);
	if (ft_is_builtin(cmd[0]))
	{
		ft_execute_builtin_in_child(d, cmd);
		exit(d->last_exit_status);
	}
	cmd_path = ft_get_cmd_path(d, cmd[0], 0);
	if (!cmd_path)
	{
		ft_putstr_fd("minishell: ", 2);
		ft_putstr_fd(cmd[0], 2);
		ft_putendl_fd(": command not found", 2);
		exit(127);
	}
	execve(cmd_path, cmd, d->evs);
	free(cmd_path);
	perror("execve");
	exit(1);
}

void	ft_child_process(t_dat *d, char ***cmd, int **fd, size_t i)
{
	t_rdr	r;
	int		len;

	if (!cmd || !cmd[i] || !cmd[i][0])
	{
		ft_cleanup_data(d);
		exit(1);
	}
	len = 0;
	while (cmd[i][len])
		len++;
	ft_memset(&r, 0, sizeof(r));
	if (!ft_validate_segment(cmd[i], 0, len))
		exit(1);
	ft_setup_io(fd, i, d->tot);
	ft_close_pipes(fd, d->tot);
	ft_parse_redirection(cmd[i], &r);
	if (!ft_apply_redirections(&r, &cmd[i]))
	{
		ft_free_redirection(&r);
		ft_cleanup_data(d);
		exit(1);
	}
	ft_exec_command(d, cmd[i]);
}

int	ft_syntax_error_msg(char *token)
{
	char	*prefix;
	char	*newline;

	prefix = "minishell: syntax error near unexpected token `";
	newline = "'\n";
	if (token)
	{
		write(2, prefix, ft_strlen(prefix));
		write(2, token, ft_strlen(token));
		write(2, newline, 2);
	}
	else
	{
		write(2, prefix, ft_strlen(prefix));
		write(2, "newline", 7);
		write(2, newline, 2);
	}
	return (0);
}

int	ft_validate_segment(char **tokens, int start, int end)
{
	int	i;

	if (!tokens || start >= end)
		return (0);
	i = start;
	while (i < end)
	{
		if (ft_is_redirection(tokens[i]))
		{
			if (i + 1 >= end || ft_is_redirection(tokens[i + 1])
				|| !ft_strcmp(tokens[i + 1], "|"))
			{
				return (ft_syntax_error_msg(tokens[i + 1]));
			}
		}
		i++;
	}
	return (1);
}

void	ft_fork_children(t_dat *d, char ***cmd, int **fd)
{
	pid_t	pid;
	size_t	i;

	i = 0;
	while (i < d->tot)
	{
		if (!cmd[i] || !cmd[i][0])
		{
			i++;
			continue ;
		}
		pid = fork();
		if (pid == 0)
			ft_child_process(d, cmd, fd, i);
		else if (pid < 0)
			perror("fork");
		i++;
	}
}

void	ft_close_pipes(int **fd, int tot)
{
	int	i;

	i = 0;
	while (i < tot - 1)
	{
		close(fd[i][0]);
		close(fd[i][1]);
		i++;
	}
}

void	ft_wait_children(t_dat *d, int tot)
{
	int	status;
	int	i;

	i = 0;
	while (i < tot)
	{
		wait(&status);
		if (WIFEXITED(status))
			d->last_exit_status = WEXITSTATUS(status);
		i++;
	}
}

void	ft_execute_pipeline(t_dat *d, char ***cmd)
{
	int	**fd;

	ft_set_child_signals();
	fd = init_fd_array(d->tot);
	if (!fd || !ft_create_pipes(fd, d->tot))
	{
		if (fd)
			ft_free_fd(fd);
		return ;
	}
	ft_fork_children(d, cmd, fd);
	ft_close_pipes(fd, d->tot);
	ft_wait_children(d, d->tot);
	ft_set_main_signals();
	ft_free_fd(fd);
}

int	ft_parse_redirection(char **tokens, t_rdr *r)
{
	int	i;

	i = 0;
	ft_memset(r, 0, sizeof(*r));
	while (tokens[i])
	{
		if (!ft_strcmp(tokens[i], "<") && tokens[i + 1])
			r->in_file = ft_strdup(tokens[++i]);
		else if (!ft_strcmp(tokens[i], ">") && tokens[i + 1])
			r->out_file = ft_strdup(tokens[++i]);
		else if (!ft_strcmp(tokens[i], ">>") && tokens[i + 1])
			r->append_file = ft_strdup(tokens[++i]);
		else if (!ft_strcmp(tokens[i], "<<") && tokens[i + 1])
			r->heredoc_delim = ft_strdup(tokens[++i]);
		i++;
	}
	return (1);
}

int	ft_redir_in(char *file)
{
	int	fd;

	fd = open(file, O_RDONLY);
	if (fd < 0)
		return (perror(file), 0);
	if (dup2(fd, STDIN_FILENO) < 0)
		return (perror("dup2 in"), 0);
	close(fd);
	return (1);
}

int	ft_redir_out(char *file)
{
	int	fd;

	fd = open(file, O_CREAT | O_WRONLY | O_TRUNC, 0644);
	if (fd < 0)
		return (perror(file), 0);
	if (dup2(fd, STDOUT_FILENO) < 0)
		return (perror("dup2 out"), 0);
	close(fd);
	return (1);
}

int	ft_redir_append(char *file)
{
	int	fd;

	fd = open(file, O_CREAT | O_WRONLY | O_APPEND, 0644);
	if (fd < 0)
		return (perror(file), 0);
	if (dup2(fd, STDOUT_FILENO) < 0)
		return (perror("dup2 append"), 0);
	close(fd);
	return (1);
}

void	ft_heredoc_child(char *delim, char *line, int *pipefd)
{
	ft_set_heredoc_signals();
	close(pipefd[0]);
	while (1)
	{
		line = readline("> ");
		if (!line)
			break ;
		//		if (ft_strncmp(line, delim, strlen(delim)) == 0
		//			&& line[strlen(delim)] == '\0')
		if (ft_strcmp(line, delim) == 0)
		{
			free(line);
			break ;
		}
		write(pipefd[1], line, strlen(line));
		write(pipefd[1], "\n", 1);
		free(line);
	}
	close(pipefd[1]);
	exit(0);
}

int	ft_heredoc_parent(int *pipefd, pid_t pid, int saved_stdin)
{
	int	status;

	//	int	saved_stdin;
	saved_stdin = dup(STDIN_FILENO);
	close(pipefd[1]);
	waitpid(pid, &status, 0);
	ft_set_main_signals();
	if (WIFSIGNALED(status) && WTERMSIG(status) == SIGINT)
	{
		close(pipefd[0]);
		dup2(saved_stdin, STDIN_FILENO);
		close(saved_stdin);
		return (0);
	}
	if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
	{
		close(pipefd[0]);
		dup2(saved_stdin, STDIN_FILENO);
		close(saved_stdin);
		return (0);
	}
	dup2(pipefd[0], STDIN_FILENO);
	close(pipefd[0]);
	close(saved_stdin);
	return (1);
}

int	ft_handle_heredoc(char *delim, char *line)
{
	int		pipefd[2];
	pid_t	pid;
	int		saved_stdin;

	saved_stdin = dup(STDIN_FILENO);
	if (pipe(pipefd) == -1)
	{
		perror("pipe");
		close(saved_stdin);
		return (0);
	}
	pid = fork();
	if (pid < 0)
	{
		perror("fork");
		close(saved_stdin);
		return (0);
	}
	if (pid == 0)
		ft_heredoc_child(delim, line, pipefd);
	return (ft_heredoc_parent(pipefd, pid, saved_stdin));
}

int	ft_apply_sing_redirections(t_rdr *r, char **tok)
{
	int	result;

	result = 1;
	if (r->in_file && !ft_redir_in(r->in_file))
		result = 0;
	if (r->out_file && !ft_redir_out(r->out_file))
		result = 0;
	if (r->append_file && !ft_redir_append(r->append_file))
		result = 0;
	if (r->heredoc_delim && !ft_handle_heredoc(r->heredoc_delim, NULL))
		result = 0;
	ft_free_redirection(r);
	if (result)
		return (ft_remove_sing_redirections(tok, 0, 0));
	else
		return (0);
}

int	ft_apply_redirections(t_rdr *r, char ***cmd)
{
	if (r->in_file && !ft_redir_in(r->in_file))
		return (0);
	if (r->out_file && !ft_redir_out(r->out_file))
		return (0);
	if (r->append_file && !ft_redir_append(r->append_file))
		return (0);
	if (r->heredoc_delim && !ft_handle_heredoc(r->heredoc_delim, NULL))
		return (0);
	ft_free_redirection(r);
	return (ft_remove_redirections(cmd, 0, 0));
}

int	ft_remove_redirections(char ***tokens_ptr, int i, int j)
{
	char	**tokens;

	if (!tokens_ptr || !*tokens_ptr)
		return (0);
	tokens = *tokens_ptr;
	while (tokens[i])
	{
		if (ft_is_redirection(tokens[i]) && tokens[i + 1])
		{
			free(tokens[i]);
			free(tokens[i + 1]);
			j = i - 1;
			while (tokens[++j + 2])
				tokens[j] = tokens[j + 2];
			tokens[j] = NULL;
			tokens[j + 1] = NULL;
			continue ;
		}
		i++;
	}
	*tokens_ptr = tokens;
	return (1);
}

int	ft_remove_sing_redirections(char **t, int i, int j)
{
	if (!t)
		return (0);
	i = 0;
	while (t[i])
	{
		if (ft_is_redirection(t[i]) && t[i + 1])
		{
			free(t[i]);
			free(t[i + 1]);
			j = i;
			while (t[j + 2])
			{
				t[j] = t[j + 2];
				j++;
			}
			t[j] = NULL;
			t[j + 1] = NULL;
			continue ;
		}
		i++;
	}
	return (1);
}

int	ft_is_redirection(char *str)
{
	return (!ft_strcmp(str, "<") || !ft_strcmp(str, ">") || !ft_strcmp(str,
			">>") || !ft_strcmp(str, "<<"));
}

void	ft_free_redirection(t_rdr *r)
{
	if (r->in_file)
	{
		free(r->in_file);
		r->in_file = NULL;
	}
	if (r->out_file)
	{
		free(r->out_file);
		r->out_file = NULL;
	}
	if (r->append_file)
	{
		free(r->append_file);
		r->append_file = NULL;
	}
	if (r->heredoc_delim)
	{
		free(r->heredoc_delim);
		r->heredoc_delim = NULL;
	}
}

int	ft_syntax_error(char *token)
{
	char	*mes1;
	char	*mes2;

	mes1 = "minishell: syntax error near unexpected token `";
	mes2 = "minishell: syntax error near unexpected token `newline'\n";
	if (token)
	{
		write(2, mes1, ft_strlen(mes1));
		write(2, token, ft_strlen(token));
		write(2, "'\n", 2);
	}
	else
		write(2, mes2, ft_strlen(mes2));
	return (0);
}

int	ft_check_redir(char **tokens, int i)
{
	if (!tokens[i + 1] || ft_is_redirection(tokens[i + 1])
		|| !ft_strcmp(tokens[i + 1], "|"))
		return (ft_syntax_error(tokens[i + 1]));
	return (1);
}

int	ft_check_pipe(char **tokens, int i)
{
	if (i == 0 || !tokens[i + 1] || ft_is_redirection(tokens[i + 1]))
		return (ft_syntax_error(tokens[i + 1]));
	return (1);
}

int	ft_validate_syntax(char **tokens)
{
	int	i;

	if (!tokens || !tokens[0])
		return (0);
	if (ft_is_redirection(tokens[0]))
		return (0);
	i = 0;
	while (tokens[i])
	{
		if (ft_is_redirection(tokens[i]))
		{
			if (!ft_check_redir(tokens, i))
				return (0);
		}
		else if (!ft_strcmp(tokens[i], "|"))
		{
			if (!ft_check_pipe(tokens, i))
				return (0);
		}
		i++;
	}
	return (1);
}

char	*ft_strjoin_free(char *s1, char *s2)
{
	char	*joined;

	if (!s1 || !s2)
		return (NULL);
	joined = ft_strjoin(s1, s2);
	free(s1);
	return (joined);
}

char	*append_exit_status(char *res, int status, int *i)
{
	char	*temp;

	temp = ft_itoa(status);
	res = ft_strjoin_free(res, temp);
	free(temp);
	*i += 2;
	return (res);
}

char	*append_char(char *res, char *token, int *i)
{
	char	*temp;

	temp = ft_substr(token, *i, 1);
	res = ft_strjoin_free(res, temp);
	free(temp);
	(*i)++;
	return (res);
}

char	*ft_expand_exit_status(t_dat *d, char *token)
{
	char	*res;
	int		i;

	i = 0;
	res = malloc(1);
	if (!res)
		return (NULL);
	res[0] = '\0';
	while (token[i])
	{
		if (token[i] == '$' && token[i + 1] == '?')
			res = append_exit_status(res, d->last_exit_status, &i);
		else
			res = append_char(res, token, &i);
	}
	return (res);
}

/*
void	ft_preprocess_tokens(t_dat *data, int len, int i)
{
	char	**new_xln;

	if (!data->xln || !data->xln[0])
		return ;
	if (ft_is_redirection(data->xln[0]) && !data->xln[1])
		return ;
	if (ft_is_redirection(data->xln[0]) || ft_is_redirection(data->xln[1]))
	{
		len = 0;
		while (data->xln[len])
			len++;
		new_xln = malloc(sizeof(char *) * (len + 2));
		if (!new_xln)
			return ;
		new_xln[0] = ft_strdup("cat");
		i = -1;
		while (++i < len)
			new_xln[i + 1] = ft_strdup(data->xln[i]);
		new_xln[len + 1] = NULL;
		ft_free_string_array(data->xln);
		data->xln = new_xln;
	}
}
*/

/*
run with the following command
cc -Wall -Werror -Wextra -o minishell minishell.c ./libft/libft.a -lreadline
HA="their $HOME" hu="their $home" GO=slow BALL='balance them $HOME'
HAD="their $HOME" hug="their $home" GO=slower BALL='balance "as" them $HOME'

*/
