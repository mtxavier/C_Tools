define(DocBegin,dnl
{<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html
><head
	><meta 
		content="text/html; charset=UTF-8"
		http-equiv="Content-Type"
	><title>}{$1}dnl
	{</title
></head
><body>
})dnl
define(DocEnd,{</body></html>})dnl
define({hdr_},{{<h}{$1}{>}{$2}{</h}$1{>}})dnl
define({href_},{{<a href="#}{$1}{">}{$2}{</a>}})dnl
define({ref_},{{<a name="}{$1}{">}{$2}{</a>}})dnl
define({tab_},{<div style="margin-left: eval(}{$1}{*40)px;">}{$2}{</div>})dnl
define({par_},{ifelse($#,0,<br>
    ,$#,1,{$1}<br>
    ,{$1}<br>
    {par_(shift($@))})})dnl
define({_trw_},{ifelse($#,0,,$#,1,<td>{$1}</td>,<td>{$1}</td>{_trw_(shift($@))})})dnl
define({trw_},{ifelse({$1},{},{<tr>}{_trw_(shift($@))}{</tr>},{<tr><th>{$1}</th>}{_trw_(shift($@))}{</tr>})})dnl
define({thd_},{ifelse($#,0,,$#,1,<th>{$1}</th>,<th>{$1}</th>{thd_(shift($@))})})dnl
define({_table_body_},{ifelse($#,0,,$#,1,{trw_($1)},{trw_($1)}{_table_body_(shift($@))})})dnl
define({table_body_},{ifelse({$1},{},{_table_body_(shift($@))},{thd_($1)}{_table_body_(shift($@))})})dnl
define({table_},{<table border="1"><tbody>}{table_body_($@)}{</tbody></table>})dnl
define({_li_},{ifelse($#,0,,$#,1,<li>{$1}</li>,
<li>{$1}</li>
{_li_(shift($@))})})dnl
define({list_},{<ul>}{_li_($@)}{</ul>})dnl
define({list0_},{<ol>}{_li_($@)}{</ol>})dnl
define({br_},{<br>})dnl
define({b_},{<b>}{$1}{</b>})dnl
define({i_},{<i>}{$1}{</i>})dnl
