(Node2D Name="SearchContent"
  (Grid
    (StackView Name="SearchResults" Spacing=14 Overflow="hidden scroll" JustifyContent="Start"
      (StackView Name="SearchSuggestions" Visible=(bind "DataContext/SearchPosts/IsIdle") Spacing=14 JustifyContent="Start"
        (TextBlock Text="trending tags" FontSize=20 class="octopus-accent-text" IgnoreHitTest=true)
        (ItemsStack Name="TrendingTags" Spacing=14 JustifyContent="Start" ItemNamePrefix="Trending" ItemsSource=(bind "DataContext/Trending/Items") ItemTemplate="Octopus/Templates/TrendingTag")
        (TextBlock Text="Search post titles and people, or start with # to browse a tag." FontSize=13 WordWrap=true MarginTop=8 class="octopus-dim" IgnoreHitTest=true))
      (StackView Name="SearchPeopleSection" Visible=(bind "DataContext/SearchPeople/HasItems") Spacing=14 JustifyContent="Start"
        (TextBlock Text="people" FontSize=20 class="octopus-accent-text" IgnoreHitTest=true)
        (ItemsStack Name="PeopleItems" Spacing=14 JustifyContent="Start" ItemNamePrefix="Person" ItemsSource=(bind "DataContext/SearchPeople/Items") ItemTemplate="Octopus/Templates/Person"))
      (StackView Name="SearchPostsSection" Visible=(bind "DataContext/SearchPosts/HasItems") Spacing=14 JustifyContent="Start"
        (TextBlock Text=(bind "DataContext/Search/Heading") FontSize=20 MarginTop=8 class="octopus-accent-text" IgnoreHitTest=true)
        (ItemsStack Name="SearchPosts" Spacing=12 JustifyContent="Start" ItemNamePrefix="Post" ItemsSource=(bind "DataContext/SearchPosts/Items") ItemTemplate="Octopus/Templates/PostCard"))
      (StackView Name="PeopleSearchError" Visible=(bind "DataContext/SearchPeople/ShowError") Spacing=8 JustifyContent="Start"
        (StackView Spacing=8 Padding="0 0 24 0" JustifyContent="Start"
          (TextBlock Text="can't load people" FontSize=22 WordWrap=true class="octopus-muted" IgnoreHitTest=true)
          (TextBlock Text=(bind "DataContext/SearchPeople/ErrorMessage") FontSize=14 WordWrap=true class="octopus-dim" IgnoreHitTest=true))
        (Button Name="RetryPeopleSearch" Click="SearchPeople.Retry" Height=48 FontSize=15 TextHorizontalAlignment="Left" TextVerticalAlignment="Center" class="octopus-action octopus-link" "retry search"))
      (StackView Name="SearchFailed" Visible=(bind "DataContext/SearchPosts/ShowError") Spacing=8 Padding="0 0 24 0" JustifyContent="Start"
        (TextBlock Text="search failed" FontSize=22 WordWrap=true class="octopus-muted" IgnoreHitTest=true)
        (TextBlock Text=(bind "DataContext/SearchPosts/ErrorMessage") FontSize=14 WordWrap=true class="octopus-dim" IgnoreHitTest=true))
      (StackView Name="NothingFound" Spacing=8 Padding="0 0 24 0" JustifyContent="Start" Visible=(and (bind "DataContext/SearchPosts/ShowEmpty") (bind "DataContext/SearchPeople/IsEmpty"))
        (TextBlock Text="nothing found" FontSize=22 WordWrap=true class="octopus-muted" IgnoreHitTest=true)
        (TextBlock Text=(bind "DataContext/Search/Missing") FontSize=14 WordWrap=true class="octopus-dim" IgnoreHitTest=true)))
    (Grid Name="Searching" IgnoreHitTest=true Visible=(or (bind "DataContext/SearchPosts/IsBusy") (bind "DataContext/SearchPeople/IsBusy"))
      (ActivityIndicator Name="SearchingIndicator" class="octopus-dim")))
  (Node2D Name="SearchBar" SafeAreaEdge="Top" MarginLeft=-20 MarginRight=-20 Padding="20 0 20 16" class="frosted bar-bottom"
    (Grid Columns="1fr 48px" Height=48
      (Input Name="SearchInput" PlaceholderText="Posts, people or #tags" Height=44 FontSize=15 Padding="12 0" BorderWidth=2 Submit="RunSearch" class="octopus-input octopus-field")
      (Button Name="RunSearch" Click="RunSearch" Width=48 Height=48 class="octopus-action octopus-primary"
        (VectorView Source="Octopus/assets/icons/search.svg" Width=20 Height=20 MarginLeft="auto" MarginRight="auto" MarginTop="auto" MarginBottom="auto" class="octopus-on-accent" IgnoreHitTest=true))))
  (Node2D Name="SearchStatusBar" SafeAreaEdge="Bottom" Visible=false MarginLeft=-20 MarginRight=-20 Padding="20 0" class="frosted bar-top"
    (TextBlock Name="SearchStatus" FontSize=11 class="octopus-dim" IgnoreHitTest=true)))
